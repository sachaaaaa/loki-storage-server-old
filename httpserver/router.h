#pragma once

#include "request_handler.h"
#include "types.h"

#include <boost/optional.hpp>

namespace boost {
    namespace asio {
        class io_context;
    }
}
class Database;

namespace loki {

class Swarm;
class message_notifier;

class router {
  public:
    router(boost::asio::io_context& ioc, Database& db, message_notifier& notifier, Swarm& swarm);

    void handle(connection_ptr& conn);

  private:
    request_handler root_handler_;
};
} // namespace loki
