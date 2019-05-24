#include "snode_push_api.h"
#include "Database.hpp"
#include "common.h"
#include "connection.h"
#include "message_notifier.h"
#include "serialization.h"

#include <cassert>

namespace loki {
snode_push_api::snode_push_api(std::string path, Database& db, message_notifier& notifier)
    : snode_api(path), db_(db), notifier_(notifier) {}

bool snode_push_api::full_match(std::string uri) {
    return partial_match(uri) != boost::none;
}

void snode_push_api::handle_snode_request(connection_ptr& conn) {
    const auto uri = conn->request().target().to_string();
    const auto path = *partial_match(uri);

    if (path == "/push") {
        handle_push(conn);
    } else if (path == "/push_batch") {
        handle_push_batch(conn);
    } else {
        conn->not_found();
    }
}

void snode_push_api::handle_push(connection_ptr& conn) {
    /// NOTE:: we only expect one message here, but
    /// for now lets reuse the function we already have
    std::vector<message_ptr> messages =
        deserialize_messages(conn->request().body());

    if (messages.size() == 0) {
        conn->bad_request("Could not deserialize body");
        return;
    }

    const auto& msg = messages.front();

    // TODO: have another layer between handlers and db that manages the
    // notifier
    if (db_.store(msg->hash, msg->pub_key, msg->data, msg->ttl, msg->timestamp,
                  msg->nonce)) {
        notifier_.notify(msg->pub_key, msg);
        //        BOOST_LOG_TRIVIAL(debug) << "saved message: " << msg.data;
    }
    // swarm->propagate....

    conn->do_write();
}

void snode_push_api::handle_push_batch(connection_ptr& conn) {
    conn->response().body() = "pushing batch";
    conn->do_write();
}
} // namespace loki
