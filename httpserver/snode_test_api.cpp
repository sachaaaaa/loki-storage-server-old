#include "snode_test_api.h"
#include "connection.h"
#include "../external/json.hpp"

namespace loki {

snode_test_api::snode_test_api(std::string path) : snode_api(path) {}

void snode_test_api::handle_snode_request(connection_ptr& conn) {
    using nlohmann::json;
    
    const json body = json::parse(conn->request().body(), nullptr, false);
    
    if (body == nlohmann::detail::value_t::discarded) {
//        BOOST_LOG_TRIVIAL(error)
//        << "Bad snode test request: invalid json";
        conn->bad_request("Bad snode test request: invalid json");
        return;
    }
    
    uint64_t blk_height;
    std::string msg_hash;
    
    try {
        blk_height = body.at("height").get<uint64_t>();
        msg_hash = body.at("hash").get<std::string>();
    } catch (...) {
//        BOOST_LOG_TRIVIAL(error)
//        << "Bad snode test request: missing fields in json";
        conn->bad_request("Bad snode test request: missing fields in json");
        return;
    }
    
    std::string tester_pk;
#ifndef DISABLE_SNODE_SIGNATURE
    // Note we know that the header is present because we already
    // verified the signature (how can we enforce that in code?)
    tester_pk = header_.at(LOKI_SENDER_SNODE_PUBKEY_HEADER);
    tester_pk.append(".snode");
#endif
    // TODO: just do
//    snode_test_.process_message_test_req(blk_height, tester_pk, msg_hash);
    conn->do_write();
}
}
