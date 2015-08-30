#include <cstdlib>
#include <iostream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "udp_server.hpp"

udp_server::udp_server(boost::asio::io_service& io_service, short port)
  : io_service_(io_service),
    socket_(io_service, udp::endpoint(udp::v4(), port))
{
  socket_.async_receive_from(
      boost::asio::buffer(&answer[0], sizeof(answer[0])), sender_endpoint_,
      boost::bind(&udp_server::handle_receive_from, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void udp_server::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd)
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
  answer[1] = htobe64(b);
  if (!error && bytes_recvd > 0)
  {
    socket_.async_send_to(
        boost::asio::buffer(answer, sizeof(answer)), sender_endpoint_,
        boost::bind(&udp_server::handle_send_to, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
  else
  {
    socket_.async_receive_from(
        boost::asio::buffer(&answer[0], sizeof(answer[0])), sender_endpoint_,
        boost::bind(&udp_server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}

void udp_server::handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
{
  if (!error && bytes_sent > 0)
  {
    socket_.async_receive_from(
        boost::asio::buffer(answer, sizeof(answer)), sender_endpoint_,
        boost::bind(&udp_server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}
