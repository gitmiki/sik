//
// async_udp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "config.hpp"

using boost::asio::ip::udp;

class server
{
public:
  server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service),
      socket_(io_service, udp::endpoint(udp::v4(), port))
  {
    answer[1] = 666;
    socket_.async_receive_from(
        boost::asio::buffer(answer, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd)
  {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
    answer[1] = htobe64(b);
    if (DEBUG)
      std::cout << "Sending back time " << b << " as " << answer[1] << std::endl;
    if (!error && bytes_recvd > 0)
    {
      socket_.async_send_to(
          boost::asio::buffer(answer, max_length), sender_endpoint_,
          boost::bind(&server::handle_send_to, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      socket_.async_receive_from(
          boost::asio::buffer(answer, max_length), sender_endpoint_,
          boost::bind(&server::handle_receive_from, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }

  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
  {
    socket_.async_receive_from(
        boost::asio::buffer(answer, max_length), sender_endpoint_,
        boost::bind(&server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  enum { max_length = 1024 };
  //boost::uint64_t answer[0];
  boost::uint64_t answer[2];
  //char [max_length];
};
