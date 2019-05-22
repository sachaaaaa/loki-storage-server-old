#pragma once

#include "request.h"
#include "response.h"

#include <boost/beast/http.hpp>

#include <functional>

namespace loki {

class json_rpc_api {
  public:
    void operator()(const request_t& req, response_t& res, std::function<void()> done) {
        if (req.method() != boost::beast::http::verb::post) {
            res.result(boost::beast::http::status::bad_request);
            return;
        }
        handle(req, res, std::move(done));
    }
    virtual void handle(const request_t& req, response_t& res, std::function<void()>) = 0;
};

} // namespace loki
