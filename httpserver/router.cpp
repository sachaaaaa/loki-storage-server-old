#include "router.h"
#include "connection.h"
#include "message_notifier.h"
#include "swarm.h"
#include "client_api.h"
#include "snode_push_api.h"
#include "snode_test_api.h"


#include <boost/asio.hpp>

namespace loki {

    router::router(boost::asio::io_context& ioc, Database& db, message_notifier& notifier, Swarm& swarm) : root_handler_() {
        root_handler_.add<client_api>("/v1/storage_rpc", db, notifier, swarm);
        root_handler_.add<snode_push_api>("/v1/swarms", db, notifier);
        root_handler_.add<snode_test_api>("/msg_test");
    }

    void router::handle(connection_ptr& conn) {
        const auto uri = conn->request().target().to_string();
        if (auto handler = root_handler_.parse(uri)) {
            (*handler).handle_request(conn);
            return;
        }
        conn->not_found();
    }
}
