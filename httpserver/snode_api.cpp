#include "snode_api.h"
#include "connection.h"

#include <boost/optional.hpp>
namespace loki {

constexpr auto LOKI_SENDER_SNODE_PUBKEY_HEADER = "X-Loki-Snode-PubKey";
constexpr auto LOKI_SNODE_SIGNATURE_HEADER = "X-Loki-Snode-Signature";

snode_api::snode_api(std::string path) : request_handler(path) {}

static boost::optional<std::string> read_header(const request_t& request, std::string key) {
    const auto it = request.find(key);
    if (it == request.end()) {
        return boost::none;
    }
    return it->value().to_string();
}
void snode_api::handle_request(connection_ptr& conn) {
#ifndef DISABLE_SNODE_SIGNATURE
    const auto pubkey = read_header(conn->request(), LOKI_SENDER_SNODE_PUBKEY_HEADER);
    const auto signature = read_header(conn->request(), LOKI_SNODE_SIGNATURE_HEADER);
    if (!pubkey || !signature) {
        conn->bad_request("Missing signature and pubkey in headers");
        return;
    }
    // if (!verify_signature()) {
    //     conn->bad_request("Could not validate signature from snode");
    //     return;
    // }
#endif
    handle_snode_request(conn);
}
}
