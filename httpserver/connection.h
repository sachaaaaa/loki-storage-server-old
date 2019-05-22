#pragma once

#include "request.h"
#include "request_handler.h"
#include "response.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/optional.hpp>

#include <array>
#include <functional>
#include <memory>

namespace loki {

class connection_manager;

/// Represents a single connection from a client.
class connection : public std::enable_shared_from_this<connection> {
  public:
    connection(const connection&) = delete;
    connection& operator=(const connection&) = delete;

    /// Construct a connection with the given socket.
    explicit connection(boost::asio::ip::tcp::socket socket,
                        connection_manager& manager, request_handler& handler);

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

  private:
    /// Perform an asynchronous read operation.
    void do_read();

    /// Perform an asynchronous write operation.
    void do_write();

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    connection_manager& connection_manager_;

    /// The handler used to process the incoming request.
    request_handler& request_handler_;

    /// The incoming request.
    request_t request_;

    /// The reply to be sent back to the client.
    response_t reply_;

    boost::beast::flat_buffer buffer_;

    router root_;
};

typedef std::shared_ptr<connection> connection_ptr;

} // namespace loki
