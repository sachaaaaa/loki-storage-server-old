#pragma once

#include "request_handler.h"
#include "json_rpc_api.h"
#include "types.h"

#include <string>

class Database;

namespace loki {

class Swarm;
class message_notifier;

class client_api : public json_rpc_handler {
  public:
    client_api(std::string path, Database& db, message_notifier& notifier, Swarm& swarm);

    virtual void handle_json_rpc(connection_ptr& conn, json::iterator method_it, json::iterator params_it) override;
private:
    void handle_store(connection_ptr& conn, json::iterator params_it);
    void handle_retrieve(connection_ptr& conn, json::iterator params_it);
    void handle_get_snode_for_pubkey(connection_ptr& conn, json::iterator params_it);
    void handle_wrong_swarm(connection_ptr& conn, const std::string& pk);
    
    Database& db_;
    message_notifier& notifier_;
    Swarm& swarm_;
};

} // namespace loki
