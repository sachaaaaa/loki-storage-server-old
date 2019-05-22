#include "router.h"

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/trim.hpp>

namespace loki {
    router::router(std::string slug, callback_t cb) : controller_(cb), slug_(slug) {
    }
    
    router& router::add(std::string slug, callback_t cb) {
        sub_controllers_.emplace_back(slug_ + slug, std::move(cb));
        return sub_controllers_.back();
    }
    
bool router::matches(std::string uri) {
    if (uri.length() < slug_.length())
        return false;

    if (uri.compare(0, slug_.length(), slug_) != 0)
        return false;

    // uri should continue with a /
    if (uri.length() > slug_.length() && uri[slug_.length()] != '/')
        return false;

    return true;
}

boost::optional<controller&> router::parse(std::string uri) {

        boost::trim_right_if(uri, boost::is_any_of("/"));

        if (!matches(uri))
            return boost::none;

        for (auto& sub_controller : sub_controllers_) {
            if (auto opt_controller = sub_controller.parse(uri))
                return *opt_controller;
        }

        if (uri == slug_ && controller_)
            return *controller_;

        return boost::none;
    }

} // namespace loki
