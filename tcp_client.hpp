#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class tcp_client
{
public:
  tcp_client(boost::asio::io_service& io_service,
      const std::string& server);

private:
  boost::posix_time::ptime t1;
  boost::posix_time::ptime t2;
  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
};

#endif /* TCP_CLIENT_HPP */
