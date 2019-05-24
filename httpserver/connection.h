#pragma once

#include "types.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/optional.hpp>

#include <array>
#include <functional>
#include <memory>

namespace loki {

class router;
class connection_manager;

/// Represents a single connection from a client.
class connection : public std::enable_shared_from_this<connection> {
  using read_handler_t = std::function<void(boost::system::error_code, std::size_t)>;
  public:
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /// Construct a connection with the given socket.
    explicit connection(boost::asio::ip::tcp::socket socket,
                        connection_manager& manager, router& router);

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

    /// Perform an asynchronous write operation.
    void do_write();

    void on_read(read_handler_t&&);

    const request_t& request() const;

    response_t& response();

    void bad_request(const std::string& message = "");
    void not_found(const std::string& message = "");
    void not_acceptable(const std::string& message = "");

  private:
    /// Perform an asynchronous read operation.
    void do_read();

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    connection_manager& connection_manager_;

    /// The incoming request.
    request_t request_;

    /// The reply to be sent back to the client.
    response_t reply_;

    boost::beast::flat_buffer buffer_;

    read_handler_t read_handler_;
};

typedef std::shared_ptr<connection> connection_ptr;

} // namespace loki
