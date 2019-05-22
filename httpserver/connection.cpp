#include "connection.h"
#include "connection_manager.h"
#include "request_handler.h"

#include <boost/beast/http.hpp>

#include <utility>
#include <vector>
#include <string>

namespace loki {

connection::connection(boost::asio::ip::tcp::socket socket,
    connection_manager& manager, request_handler& handler)
  : socket_(std::move(socket)),
    connection_manager_(manager),
    request_handler_(handler)
{
}

void connection::start()
{
  do_read();
}

void connection::stop()
{
  socket_.close();
}

void connection::do_read()
{
    boost::beast::http::async_read(socket_, buffer_, request_,
      [this](boost::system::error_code ec, std::size_t bytes_transferred)
      {
        if (!ec)
        {
            request_handler_.handle_request(request_, reply_, std::bind(&connection::do_write, this));
        }
        else if (ec != boost::asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}

void connection::do_write()
{
    boost::beast::http::async_write(socket_, reply_,
      [this](boost::system::error_code ec, std::size_t)
      {
        if (!ec)
        {
          // Initiate graceful connection closure.
          boost::system::error_code ignored_ec;
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
            ignored_ec);
        }

        if (ec != boost::asio::error::operation_aborted)
        {
          connection_manager_.stop(shared_from_this());
        }
      });
}

} // namespace loki
