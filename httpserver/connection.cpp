#include "connection.h"
#include "connection_manager.h"
#include "router.h"

#include <boost/beast/http.hpp>

#include <string>
#include <utility>
#include <vector>

namespace loki {

connection::connection(boost::asio::ip::tcp::socket socket,
                       connection_manager& manager, router& router)
    : socket_(std::move(socket)), connection_manager_(manager) {}

void connection::start() { do_read(); }

void connection::stop() { socket_.close(); }

void connection::on_read(read_handler_t&& read_handler) {
    read_handler_ = std::move(read_handler);
}

void connection::do_read() {
    boost::beast::http::async_read(socket_, buffer_, request_, read_handler_);
}

void connection::do_write() {
    boost::beast::http::async_write(
        socket_, reply_,
        [this](boost::system::error_code ec, std::size_t) mutable {
            if (!ec) {
                // Initiate graceful connection closure.
                boost::system::error_code ignored_ec;
                socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                                 ignored_ec);
            }

            if (ec != boost::asio::error::operation_aborted) {
                connection_manager_.stop(shared_from_this());
            }
        });
}

const request_t& connection::request() const { return request_; }

response_t& connection::response() { return reply_; }

void connection::bad_request(const std::string& message) {
    reply_.result(boost::beast::http::status::bad_request);
    reply_.set(boost::beast::http::field::content_type, "text/plain");
    reply_.body() = message;
    do_write();
}

void connection::not_found(const std::string& message) {
    reply_.result(boost::beast::http::status::not_found);
    reply_.set(boost::beast::http::field::content_type, "text/plain");
    reply_.body() = message;
    do_write();
}

void connection::not_acceptable(const std::string& message) {
    reply_.result(boost::beast::http::status::not_acceptable);
    reply_.set(boost::beast::http::field::content_type, "text/plain");
    reply_.body() = message;
    do_write();
}

} // namespace loki
