#include "request_handler.h"
#include "connection.h"

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/trim.hpp>

namespace loki {

request_handler::request_handler(std::string path) : path_(path) {
    boost::trim_right_if(path_, boost::is_any_of("/"));
}

bool request_handler::full_match(std::string uri) {
    return uri == path_;
}

boost::optional<std::string> request_handler::partial_match(std::string uri) {
    if (uri.length() < path_.length())
        return boost::none;

    if (uri.compare(0, path_.length(), path_) != 0)
        return boost::none;

    // uri should continue with a /
    if (uri.length() > path_.length() && uri[path_.length()] != '/')
        return boost::none;

    return uri.substr(path_.length());
}

void request_handler::handle_request(connection_ptr& conn) {
    conn->not_found();
}

/// Handle a request and produce a reply.
boost::optional<request_handler&> request_handler::parse(std::string uri) {
    boost::trim_right_if(uri, boost::is_any_of("/"));

    if (full_match(uri))
        return *this;

    if (auto sub_uri_opt = partial_match(uri)) {
        const auto sub_uri = *sub_uri_opt;
        for (auto& handler : sub_handlers_) {
            if (auto opt_controller = handler->parse(sub_uri))
                return *opt_controller;
        }
    }

    return boost::none;
}

} // namespace loki
