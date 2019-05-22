#pragma once

#include "response.h"
#include "request.h"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace loki {

using completion_cb_t = std::function<void()>;
using callback_t = std::function<void(const request_t&, response_t&, completion_cb_t)>;

    /// TODO: change this ! std::function will capture everything. We need to access a live object instead.
class controller {
  public:
    controller() = default;
    controller(controller&&) = default;
    controller(const controller&) = delete;
    controller& operator=(const controller&) = delete;
    controller(callback_t callback) : callback_(std::move(callback)) {}
    void handle(const request_t& req, response_t& res, completion_cb_t done) {
        if (!callback_) {
            res.result(boost::beast::http::status::not_found);
            return;
        }
        callback_(req, res, std::move(done));
    }
  protected:
    callback_t callback_;
};

class router {
  public:
    router() = default;
    router(std::string slug, callback_t cb = nullptr);

    router(const router&) = delete;
    router(router&&) = default;
    
    router& add(std::string slug, callback_t cb = nullptr);

    boost::optional<controller&> parse(std::string uri);

private:
    bool matches(std::string uri);

    boost::optional<controller> controller_;
    std::vector<router> sub_controllers_;
    std::string slug_;
};
} // namespace loki
