#pragma once

#include <boost/beast/http.hpp>

namespace loki {
    using response_t = boost::beast::http::response<boost::beast::http::string_body>;
}
