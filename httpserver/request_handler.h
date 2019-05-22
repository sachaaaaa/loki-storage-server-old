#pragma once

#include "request.h"
#include "response.h"
#include "router.h"
#include "client_api.h"

#include <boost/asio.hpp>

#include <string>
#include <functional>

namespace loki {

class request_handler {
    using completion_cb_t = std::function<void()>;
  public:
    request_handler(boost::asio::io_context&);

    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    /// Handle a request and produce a reply.
    void handle_request(const request_t& req, response_t& rep, completion_cb_t done);

  private:
    boost::asio::io_context& io_context_;
    
//    router router_;

//    client_api capi;
};

} // namespace loki
