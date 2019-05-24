#pragma once

#include "Item.hpp"
#include "common.h"

#include <boost/asio.hpp>

#include <chrono>
#include <functional>
#include <vector>

namespace loki {

constexpr static auto LONG_POLL_TIMEOUT = std::chrono::milliseconds(10000);

class message_notifier {
    using callback_t =
        std::function<void(const std::vector<message_ptr>&)>;

  public:
    message_notifier(boost::asio::io_context& ioc) : io_context_(ioc) {}

    void register_listener(std::string pk, callback_t cb) {
        listeners_.emplace_back(io_context_, pk);
        auto& context = listeners_.back();
        context.timer.expires_after(LONG_POLL_TIMEOUT);
        context.timer.async_wait(
            [&, cb = std::move(cb)](const boost::system::error_code& ec) {
                if (ec == boost::asio::error::operation_aborted)
                    cb(context.items);
                else
                    cb({});
            });
    }
    void notify(const std::string& pk, std::vector<message_ptr> items) {
        for (auto it = listeners_.begin(); it != listeners_.end();) {
            auto& listener = *it;
            if (listener.pk == pk) {
                listener.items = items;
                listener.timer.cancel();
                it = listeners_.erase(it);
            } else {
                it++;
            }
        }
    }

    void notify(const std::string& pk, const message_ptr& msg) {
        std::vector<message_ptr> items {msg};
        notify(pk, std::move(items));
    }

  private:
    boost::asio::io_context& io_context_;
    struct notifier_context {
        explicit notifier_context(boost::asio::io_context& ioc,
                                  const std::string& pk)
            : timer(ioc), pk(pk) {}
        boost::asio::steady_timer timer;
        std::string pk;
        // TODO this should be a reference somehow to avoid copies
        std::vector<message_ptr> items;
    };
    std::vector<notifier_context> listeners_;
};
} // namespace loki
