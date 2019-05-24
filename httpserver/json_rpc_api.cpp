#include "json_rpc_api.h"
#include "connection.h"

#include <boost/beast.hpp>

namespace loki {
json_rpc_handler::json_rpc_handler(std::string path)
    : request_handler(path) {}

void json_rpc_handler::handle_request(connection_ptr& conn) {
    const auto& req = conn->request();

    if (req.method() != boost::beast::http::verb::post) {
        conn->bad_request("Invalid http method");
        return;
    }

    json body = json::parse(req.body(), nullptr, false);
    if (body == nlohmann::detail::value_t::discarded) {
        conn->bad_request("Invalid json");
        return;
    }

    auto method_it = body.find("method");
    if (method_it == body.end() || !method_it->is_string()) {
        conn->bad_request("Missing or invalid rpc method");
        return;
    }

    auto params_it = body.find("params");
    if (params_it == body.end() || !params_it->is_object()) {
        conn->bad_request("Missing or invalid rpc params");
        return;
    }

    handle_json_rpc(conn, method_it, params_it);
    return;
}
} // namespace loki
