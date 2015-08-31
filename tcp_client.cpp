#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "tcp_client.hpp"

#include "connection.hpp"
extern std::vector<Connection> connections;

tcp_client::tcp_client(boost::asio::io_service& io_service, int interval)
  : resolver_(io_service),
    socket_(io_service),
    interval_(interval),
    timer_(io_service)
{
  timer_.expires_from_now(boost::posix_time::seconds(0));
  timer_.async_wait(boost::bind(&tcp_client::start_sending, this,
    boost::asio::placeholders::error));
}

void tcp_client::start_sending(const boost::system::error_code& /*e*/) {
  while (true) {
    for(uint i = 0; i < connections.size(); i++) {
      if ((connections[i].tcp_credits > 0) && (connections[i]._ssh)) {
        connect(connections[i].ip);
        connections[i].tcp_credits--;
      }
    }
    sleep(interval_);
  }
}

void tcp_client::connect(const std::string& host) {
  tcp::resolver::query query(host, "22");
  tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);
  tcp::resolver::iterator end;
  boost::system::error_code error = boost::asio::error::host_not_found;
  boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
  while (error && endpoint_iterator != end) {
    socket_.close();
    socket_.connect(*endpoint_iterator++, error);
  }
  boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
  socket_.close();
  boost::posix_time::time_duration msdiff = t2 - t1;
  std::cout << "Połączenie się za pomocą TCP z " << host << " zajmuje " << msdiff.total_microseconds() << " mikrosekund" << std::endl;
  std::cout << "Connected!" << std::endl;
  for(uint i = 0; i < connections.size(); i++) {
    if (connections[i].ip.compare(host) == 0) {
      connections[i].ssh[connections[i].pos_tcp] = (int) msdiff.total_microseconds();
      connections[i].pos_tcp = (connections[i].pos_tcp+1)%10;
    }
  }
}
