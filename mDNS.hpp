#ifndef mDNS_HPP
#define mDNS_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <boost/algorithm/string.hpp>

#include "DNSpacket.hpp"

const int max_message_count = 10;


void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host);

class mDNS
{
public:
  mDNS(boost::asio::io_service& io_service,
      const boost::asio::ip::address& multicast_address);

  void send_query();

  void handle_send_to(const boost::system::error_code& error);

  void handle_timeout(const boost::system::error_code& error);

private:
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::deadline_timer timer_;
  int message_count_;
  std::string message_;
};

#endif /* mDNS_HPP */
