#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "config.hpp"

using boost::asio::ip::udp;

class udp_client
{
public:
  udp_client(boost::asio::io_service& io_service, const std::string& host, const std::string& port);
  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent);

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint endpoint_;
  boost::posix_time::ptime t1;
  boost::posix_time::ptime t2;
  boost::uint64_t answer[2];
};

#endif /* UDP_CLIENT_HPP */
