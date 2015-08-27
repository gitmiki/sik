#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

class udp_server
{
public:
  udp_server(boost::asio::io_service& io_service, short port);

  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd);

  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent);

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint sender_endpoint_;
  boost::uint64_t answer[2];
};


#endif /* UDP_SERVER_HPP */
