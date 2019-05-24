#include "server.h"

#include <signal.h>
#include <utility>

namespace loki {

    server::server(boost::asio::io_context& ioc, Database& db, const std::string& address, const std::string& port, Swarm& swarm)
  : io_context_(ioc),
    signals_(io_context_),
    acceptor_(io_context_),
    connection_manager_(),
    notifier_(io_context_),
    router_(io_context_, db, notifier_, swarm)
{
  // Register to handle the signals that indicate when the server should exit.
  // It is safe to register for the same signal multiple times in a program,
  // provided all registration for the specified signal is made through Asio.
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)

  do_await_stop();

  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  boost::asio::ip::tcp::resolver resolver(io_context_);
  boost::asio::ip::tcp::endpoint endpoint =
    *resolver.resolve(address, port).begin();
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  do_accept();
}

void server::run()
{
  // The io_context::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_context_.run();
}

void server::do_accept()
{
  acceptor_.async_accept(
      [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
      {
        // Check whether the server was stopped by a signal before this
        // completion handler had a chance to run.
        if (!acceptor_.is_open())
        {
          return;
        }

        if (!ec)
        {
          auto conn = std::make_shared<connection>(
              std::move(socket), connection_manager_, router_);

          conn->on_read([=](boost::system::error_code ec, int bytes_transferred) mutable {
            if (!ec)
            {
                router_.handle(conn);
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
              connection_manager_.stop(conn);
            }
          });

          connection_manager_.start(conn);
        }

        do_accept();
      });
}

void server::do_await_stop()
{
  signals_.async_wait(
      [this](boost::system::error_code /*ec*/, int /*signo*/)
      {
        // The server is stopped by cancelling all outstanding asynchronous
        // operations. Once all operations have finished the io_context::run()
        // call will exit.
        acceptor_.close();
        connection_manager_.stop_all();
      });
}

} // namespace loki
