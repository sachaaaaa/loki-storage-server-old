#pragma once

#include "connection.h"
#include "connection_manager.h"
#include "router.h"
#include "message_notifier.h"

#include <boost/asio.hpp>
#include <string>

class Database;

namespace loki {
    
class Swarm;
    
/// The top-level class of the HTTP server.
class server
{
public:
  server(const server&) = delete;
  server& operator=(const server&) = delete;

  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit server(boost::asio::io_context& ioc, Database& db, const std::string& address, const std::string& port, Swarm&);

  /// Run the server's io_context loop.
  void run();

private:
  /// Perform an asynchronous accept operation.
  void do_accept();

  /// Wait for a request to stop the server.
  void do_await_stop();

  /// The io_context used to perform asynchronous operations.
  boost::asio::io_context& io_context_;

  /// The signal_set is used to register for process termination notifications.
  boost::asio::signal_set signals_;

  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The connection manager which owns all live connections.
  connection_manager connection_manager_;

  message_notifier notifier_;

  /// The handler for all incoming requests.
  router router_;

};

} // namespace http
