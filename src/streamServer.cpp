//
// stream_server.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <iostream>
#include <memory>


using asio::local::stream_protocol;

class session
  : public std::enable_shared_from_this<session>
{
public:
  session(asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  stream_protocol::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    socket_.async_read_some(asio::buffer(data_),
                            [this](std::error_code ec,std::size_t size){
                                handle_read(ec, size);
                            });
  }

  void handle_read(const std::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
        asio::async_write(socket_,
                                 asio::buffer(data_, bytes_transferred),
                                 [this](std::error_code ec, std::size_t size){
                                     handle_write(ec);
                                 });
    }
  }

  void handle_write(const std::error_code& error)
  {
    if (!error)
    {
        socket_.async_read_some(asio::buffer(data_),
                                [this](std::error_code ec, std::size_t size){
                                    handle_read(ec, size);
                                });
    }
  }

private:
  // The socket used to communicate with the client.
  stream_protocol::socket socket_;

  // Buffer used to store data received from the client.
  std::vector<char> data_ = std::vector<char>(1024);
};

typedef std::shared_ptr<session> session_ptr;

class server
{
public:
  server(asio::io_context& io_context, const std::string& file)
    : io_context_(io_context),
      acceptor_(io_context, stream_protocol::endpoint(file))
  {
    session_ptr new_session(new session(io_context_));
    acceptor_.async_accept(new_session->socket(),
                           [this, new_session](std::error_code ec){
                               handle_accept(new_session, ec);
                           });
  }

  void handle_accept(session_ptr new_session,
      const std::error_code& error)
  {
    if (!error)
    {
      new_session->start();
    }

    new_session.reset(new session(io_context_));
    acceptor_.async_accept(new_session->socket(),
                           [this, new_session](std::error_code ec){
                               handle_accept(new_session, ec);
                           });
  }

private:
  asio::io_context& io_context_;
  stream_protocol::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: stream_server <file>\n";
      std::cerr << "*** WARNING: existing file is removed ***\n";
      return 1;
    }

    asio::io_context io_context;

    std::remove(argv[1]);
    server s(io_context, argv[1]);

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

