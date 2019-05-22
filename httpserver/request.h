#pragma once

#include <boost/beast/http.hpp>

namespace loki {
    using request_t = boost::beast::http::request<boost::beast::http::string_body>;
}
