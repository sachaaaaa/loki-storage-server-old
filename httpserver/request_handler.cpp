#include "request_handler.h"

#include <fstream>
#include <sstream>
#include <string>
#include <chrono>

namespace loki {
    
    // TODO: request_handler should be the class that is the router and the controller?
    // Currently controllers are captured and can't be modified from the outside :(

request_handler::request_handler(boost::asio::io_context& ioc) : io_context_(ioc), capi(ioc) {
    router_.add("/test");
    auto& v1 = router_.add("/v1");
    v1.add("/storage_rpc", capi);
    capi.i = 42;
}

void request_handler::handle_request(const request_t& req, response_t& res, completion_cb_t done) {
    const auto uri = req.target().to_string();

    if (auto opt_controller = router_.parse(uri)) {
        auto& controller = *opt_controller;
        controller.handle(req, res, std::move(done));
        return;
    }
    res.result(boost::beast::http::status::not_found);
    done();
}

} // namespace loki
