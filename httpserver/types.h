#pragma once
#include <boost/beast/http.hpp>
#include <memory>

namespace loki {
class connection;
struct message_t;
}

using connection_ptr = std::shared_ptr<loki::connection>;
using message_ptr = std::shared_ptr<loki::message_t>;

using response_t = boost::beast::http::response<boost::beast::http::string_body>;
using request_t = boost::beast::http::request<boost::beast::http::string_body>;

namespace boost {
    namespace asio {
        class io_context;
    }
}
