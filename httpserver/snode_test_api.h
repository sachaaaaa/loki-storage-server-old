#pragma once

#include "snode_api.h"
#include "types.h"

#include <string>

namespace loki {

class snode_test_api : public snode_api {
  public:
    snode_test_api(std::string path);

    virtual void handle_snode_request(connection_ptr& conn) override;
};

} // namespace loki
