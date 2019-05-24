#pragma once

#include "types.h"

#include <boost/beast.hpp>
#include <boost/optional.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility> // std::forward
#include <vector>

namespace loki {

class request_handler {
  public:
    request_handler(std::string path = "/");
    virtual ~request_handler() = default;

    // non-copiable
    request_handler(const request_handler&) = delete;
    request_handler& operator=(const request_handler&) = delete;

    boost::optional<request_handler&> parse(std::string uri);

    virtual void handle_request(connection_ptr& conn);

    template <typename T, typename... Args>
    request_handler& add(std::string sub_path, Args&&... args) {
        auto sub_handler =
            std::make_unique<T>(sub_path, std::forward<Args>(args)...);
        sub_handlers_.push_back(std::move(sub_handler));
        return *sub_handlers_.back();
    }

  protected:
    virtual bool full_match(std::string uri);
    boost::optional<std::string> partial_match(std::string uri);
    std::string path_;
    // multimap?
    std::vector<std::unique_ptr<request_handler>> sub_handlers_;
};

} // namespace loki
