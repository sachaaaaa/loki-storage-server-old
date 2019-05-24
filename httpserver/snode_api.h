#pragma once

#include "request_handler.h"
#include "types.h"

#include <string>

namespace loki {

class snode_api : public request_handler {
  public:
    snode_api(std::string path);

    virtual void handle_request(connection_ptr& conn) override final;
    virtual void handle_snode_request(connection_ptr& conn) = 0;
};

} // namespace loki