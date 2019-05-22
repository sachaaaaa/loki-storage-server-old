#pragma once

#include "json_rpc_api.h"

#include <boost/asio.hpp>

namespace loki {
class client_api : public json_rpc_api {
  public:
    client_api(boost::asio::io_context& ioc) : io_context_(ioc) {}

    virtual void handle(const request_t& req, response_t& res, std::function<void()> done) override {
        i++;
        boost::asio::post(io_context_, [&, done = std::move(done)]{
            res.result(boost::beast::http::status::ok);
            res.body() = "client api " + std::to_string(i);
            done();
        });
    }
    int i = 0;
private:
    boost::asio::io_context& io_context_;
    
};
} // namespace loki
