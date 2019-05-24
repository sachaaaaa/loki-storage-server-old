#include "client_api.h"
#include "Database.hpp"
#include "connection.h"
#include "message_notifier.h"
#include "pow.hpp"
#include "swarm.h"
#include "utils.hpp"

#include <boost/asio.hpp>

#include <chrono>

namespace loki {

// Note: on the client side the limit is different
// as it is not encrypted/encoded there yet.
// The choice is somewhat arbitrary but it roughly
// corresponds to the client-side limit of 2000 chars
// of unencrypted message body in our experiments
// (rounded up)
constexpr size_t MAX_MESSAGE_BODY = 3100;

client_api::client_api(std::string path, Database& db,
                       message_notifier& notifier, Swarm& swarm)
    : json_rpc_handler(path), db_(db), notifier_(notifier), swarm_(swarm) {}

void client_api::handle_json_rpc(connection_ptr& conn, json::iterator method_it,
                                 json::iterator params_it) {

    const auto& method = method_it->get_ref<std::string&>();

    if (method == "store") {
        handle_store(conn, params_it);
    } else if (method == "retrieve") {
        handle_retrieve(conn, params_it);
    } else if (method == "get_snodes_for_pubkey") {
        handle_get_snode_for_pubkey(conn, params_it);
    } else {
        conn->bad_request("Invalid rpc method");
    }
}

void client_api::handle_store(connection_ptr& conn, json::iterator params_it) {
    constexpr const char* fields[] = {"pubKey", "ttl", "nonce", "timestamp",
                                      "data"};

    auto& params = *params_it;
    auto& res = conn->response();

    for (const auto& field : fields) {
        if (!params.contains(field)) {
            conn->bad_request("Missing params field for store");
            //                body_stream_ << boost::format("invalid json: no
            //                `%1%` field\n") % field; BOOST_LOG_TRIVIAL(error)
            //                << boost::format("Bad client request: no `%1%`
            //                field") % field;
            return;
        }
    }

    std::string pubKey, ttl, nonce, timestamp, data;

    try {
        pubKey = params["pubKey"].get<std::string>();
        ttl = params["ttl"].get<std::string>();
        nonce = params["nonce"].get<std::string>();
        timestamp = params["timestamp"].get<std::string>();
        data = params["data"].get<std::string>();
    } catch (...) {
        conn->bad_request("Invalid params for store");
        return;
    }

    if (pubKey.size() != 66) {
        conn->bad_request("Pubkey must be 66 characters long");
        //            BOOST_LOG_TRIVIAL(error) << "Pubkey must be 66 characters
        //            long ";
        return;
    }

    if (data.size() > MAX_MESSAGE_BODY) {
        conn->bad_request("Message body exceeds maximum allowed length of " +
                          std::to_string(MAX_MESSAGE_BODY));
        //            BOOST_LOG_TRIVIAL(error) << "Message body too long: " <<
        //            data.size();
        return;
    }

    if (!swarm_.is_pubkey_for_us(pubKey)) {
        handle_wrong_swarm(conn, pubKey);
        return;
    }

#ifdef INTEGRATION_TEST
    BOOST_LOG_TRIVIAL(trace) << "store body: " << data;
#endif

    uint64_t ttlInt;
    if (!util::parseTTL(ttl, ttlInt)) {
        conn->not_acceptable("Provided TTL is not valid");
        //            BOOST_LOG_TRIVIAL(error) << "Forbidden. Invalid TTL " <<
        //            ttl;
        return;
    }
    uint64_t timestampInt;
    if (!util::parseTimestamp(timestamp, ttlInt, timestampInt)) {
        conn->not_acceptable("Timestamp error: check your clock");
        //            BOOST_LOG_TRIVIAL(error)
        //            << "Forbidden. Invalid Timestamp " << timestamp;
        return;
    }

    // Do not store message if the PoW provided is invalid
    std::string messageHash;

    const bool validPoW =
        checkPoW(nonce, timestamp, ttl, pubKey, data, messageHash);
#ifndef DISABLE_POW
    if (!validPoW) {
        conn->not_acceptable("Provided PoW nonce is not valid");
        //            BOOST_LOG_TRIVIAL(error) << "Forbidden. Invalid PoW nonce
        //            " << nonce;
        return;
    }
#endif

    bool saved;
    try {
        saved =
            db_.store(messageHash, pubKey, data, ttlInt, timestampInt, nonce);
    } catch (const std::exception& e) {
        res.result(boost::beast::http::status::internal_server_error);
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.body() += std::string(e.what()) + "\n";
        //            BOOST_LOG_TRIVIAL(error)
        //            << "Internal Server Error. Could not store message for "
        //            << obfuscate_pubkey(pubKey);
        return;
    }

    res.result(boost::beast::http::status::ok);
    conn->do_write();
    
    const auto msg = std::make_shared<message_t>(pubKey, std::move(data), messageHash, ttlInt, timestampInt, nonce);
    
    if (saved)
        notifier_.notify(pubKey, msg);

    // TODO: push to swarm via snode_comm?
    swarm_.propagate_message(msg);

    //        BOOST_LOG_TRIVIAL(trace)
    //        << "Successfully stored message for " << obfuscate_pubkey(pubKey);
    //        res.result(boost::beast::http::status::ok);
}

static std::string messages_to_json(const std::vector<message_ptr>& msgs) {
    using namespace nlohmann;
    json res_body;
    json messages = json::array();

    for (const auto& msg : msgs) {
        json message;
        message["hash"] = msg->hash;
        /// TODO: calculate expiration time once only?
        message["expiration"] = msg->timestamp + msg->ttl;
        message["data"] = msg->data;
        messages.push_back(message);
    }

    res_body["messages"] = messages;

    return res_body.dump();
}

void client_api::handle_retrieve(connection_ptr& conn,
                                 json::iterator params_it) {

    // TODO: should extract fields as we check + move to fcn to reuse with store
    constexpr const char* fields[] = {"pubKey", "lastHash"};

    auto& params = *params_it;
    const auto& req = conn->request();
    auto& res = conn->response();

    for (const auto& field : fields) {
        if (!params.contains(field)) {
            //                body_stream_ << boost::format("invalid json: no
            //                `%1%` field\n") % field; BOOST_LOG_TRIVIAL(error)
            //                << boost::format("Bad client request: no `%1%`
            //                field") % field;
            conn->bad_request("invalid json");
            return;
        }
    }

    const auto pub_key = params["pubKey"].get<std::string>();
    const auto last_hash = params["lastHash"].get<std::string>();

    if (!swarm_.is_pubkey_for_us(pub_key)) {
        handle_wrong_swarm(conn, pub_key);
        return;
    }

    std::vector<service_node::storage::Item> items;
    db_.retrieve(pub_key, items, last_hash);

    std::vector<message_ptr> messages;
    for (const auto& item : items) {
        messages.push_back(std::make_shared<message_t>(
            item.pub_key, std::move(item.data), item.hash, item.ttl,
            item.timestamp, item.nonce));
    }

    const bool long_polling = req.find("X-Loki-Long-Poll") != req.end();

    res.set(boost::beast::http::field::content_type, "application/json");

    if (items.size() == 0 && long_polling) {
        notifier_.register_listener(
            pub_key, [conn](const std::vector<message_ptr>& messages) {
                conn->response().result(boost::beast::http::status::ok);
                conn->response().body() = messages_to_json(messages);
                conn->do_write();
            });
    } else {
        res.result(boost::beast::http::status::ok);
        res.body() = messages_to_json(messages);
        conn->do_write();
    }
}

void client_api::handle_get_snode_for_pubkey(connection_ptr& conn,
                                             json::iterator params_it) {
    auto& res = conn->response();
    const auto& params = *params_it;

    if (!params.contains("pubKey")) {
        conn->bad_request("invalid json: no `pubKey` field");
        //        BOOST_LOG_TRIVIAL(error) << "Bad client request: no `pubKey`
        //        field";
        return;
    }

    const auto pubKey = params["pubKey"].get<std::string>();

    if (pubKey.size() != 66) {
        conn->bad_request("Pubkey must be 66 characters long");
        //        BOOST_LOG_TRIVIAL(error) << "Pubkey must be 66 characters long
        //        ";
        return;
    }

    std::vector<sn_record_t> nodes = swarm_.get_snodes_by_pk(pubKey);

    json res_body;

    json snodes = json::array();

    for (const auto& sn : nodes) {
#ifdef INTEGRATION_TEST
        snodes.push_back(std::to_string(sn.port));
#else
        snodes.push_back(sn.address);
#endif
    }

    res_body["snodes"] = snodes;

    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_type, "application/json");

    /// This might throw if not utf-8 endoded
    res.body() = res_body.dump();
    conn->do_write();
}
    
void client_api::handle_wrong_swarm(connection_ptr& conn, const std::string& pk) {
    const auto nodes = swarm_.get_snodes_by_pk(pk);
    
    json res_body;
    json snodes = json::array();
    
    for (const auto& sn : nodes) {
#ifdef INTEGRATION_TEST
        snodes.push_back(std::to_string(sn.port));
#else
        snodes.push_back(sn.address);
#endif
    }
    
    res_body["snodes"] = snodes;
    
    auto& res = conn->response();

    res.result(boost::beast::http::status::misdirected_request);
    res.set(boost::beast::http::field::content_type, "application/json");
    
    /// This might throw if not utf-8 endoded
    res.body() = res_body.dump();
    
    conn->do_write();
//    BOOST_LOG_TRIVIAL(info) << "Client request for different swarm received";
}

} // namespace loki
