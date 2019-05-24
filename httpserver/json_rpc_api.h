#pragma once

#include "request_handler.h"
#include "../external/json.hpp"
#include "types.h"

#include <boost/optional.hpp>

namespace loki {

class json_rpc_handler : public request_handler {
  public:
    using json = nlohmann::json;

    json_rpc_handler(std::string path);
    virtual ~json_rpc_handler() = default;

    void handle_request(connection_ptr& conn) override final;
    virtual void handle_json_rpc(connection_ptr& conn, json::iterator method_it, json::iterator params_it) = 0;

};

} // namespace loki
