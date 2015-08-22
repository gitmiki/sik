#include <cstdlib>
#include <iostream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "config.hpp"

using boost::asio::ip::udp;

class udp_server
{
public:
  udp_server(boost::asio::io_service& io_service, short port)
    : io_service_(io_service),
      socket_(io_service, udp::endpoint(udp::v4(), port))
  {
    socket_.async_receive_from(
        boost::asio::buffer(&answer[0], sizeof(answer[0])), sender_endpoint_,
        boost::bind(&udp_server::handle_receive_from, this,
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
      std::cout << "SERVER: Sending back time " << b << std::endl;
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

  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
  {
    socket_.async_receive_from(
        boost::asio::buffer(answer, sizeof(answer)), sender_endpoint_,
        boost::bind(&udp_server::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  boost::uint64_t answer[2];
};
