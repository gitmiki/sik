#include <iostream>
#include <sstream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "DNSpacket.hpp"

const int max_message_count = 10;

class mDNSsender
{
public:
  mDNSsender(boost::asio::io_service& io_service,
      const boost::asio::ip::address& multicast_address)
    : endpoint_(multicast_address, 5353),
      socket_(io_service, endpoint_.protocol()),
      timer_(io_service),
      message_count_(0)
  {
    DNSpacket packet;
    packet.header.ID = (unsigned short) htons (getpid());
    packet.header.qr = 0;
    packet.header.opcode = 0;
    packet.header.aa = 0;
    packet.header.tc = 0;
    packet.header.rd = 1;
    packet.header.ra = 0;
    packet.header.z = 0;
    packet.header.ad = 0;
    packet.header.cd = 0;
    packet.header.rcode = 0;
    packet.header.qdcount = htons(1);
    packet.header.ancount = 0;
    packet.header.nscount = 0;
    packet.header.arcount = 0;
    //packet.query.name = "_printer._udp.local.";
    packet.query.qtype = htons(1);
    packet.query.qclass = htons(1);


    //std::ostringstream os;
    //os << "Message " << message_count_++;
    //message_ = os.str();

    socket_.async_send_to(
        boost::asio::buffer(&packet, 1024), endpoint_,
        boost::bind(&mDNSsender::handle_send_to, this,
          boost::asio::placeholders::error));
  }

  void handle_send_to(const boost::system::error_code& error)
  {
    if (!error && message_count_ < max_message_count)
    {
      timer_.expires_from_now(boost::posix_time::seconds(1));
      timer_.async_wait(
          boost::bind(&mDNSsender::handle_timeout, this,
            boost::asio::placeholders::error));
    }
  }

  void handle_timeout(const boost::system::error_code& error)
  {
    /*if (!error)
    {
      std::ostringstream os;
      os << "Message " << message_count_++ << "\n";
      message_ = os.str();

      socket_.async_send_to(
          boost::asio::buffer(message_), endpoint_,
          boost::bind(&mDNSsender::handle_send_to, this,
            boost::asio::placeholders::error));
    }*/
  }

private:
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::deadline_timer timer_;
  int message_count_;
  std::string message_;
};
