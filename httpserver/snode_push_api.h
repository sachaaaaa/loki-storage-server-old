#pragma once

#include "snode_api.h"
#include "types.h"

#include <string>

class Database;

namespace loki {

class message_notifier;

class snode_push_api : public snode_api {
  public:
    snode_push_api(std::string path, Database& db, message_notifier& notifier);

    virtual void handle_snode_request(connection_ptr& conn) override;
    virtual bool full_match(std::string uri) override;

  private:
    void handle_push(connection_ptr& conn);
    void handle_push_batch(connection_ptr& conn);

    Database& db_;
    message_notifier& notifier_;
};

} // namespace loki
