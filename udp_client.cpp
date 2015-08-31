#include <cstdlib>
#include <iostream>
#include <ctime>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "connection.hpp"
#include "udp_client.hpp"

extern std::vector<Connection> connections;

const int MAX_TIME_FOR_RESPONSE = 2;

udp_client::udp_client(boost::asio::io_service& io_service, const std::string& host, const std::string& port, int interval)
  : io_service_(io_service),
    socket_(io_service, udp::endpoint(udp::v4(), 0)),
    interval_(interval),
    host_(host),
    port_(port),
    timer_(io_service)
{
  udp::resolver resolver(io_service_);
  udp::resolver::query query(udp::v4(), host, port_);
  udp::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;
  send();
}

void udp_client::send() {
  uint i = 0;
  for(i = 0; i < connections.size(); i++) {
    if (connections[i].ip.compare(host_) == 0) {
      if (connections[i].credits == 0) {
        connections[i].alive = false;
        i = connections.size();
      }
    break;
    }
  }
  if (i != connections.size()) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
    boost::uint64_t buff = htobe64(b);
    t1 = boost::posix_time::microsec_clock::local_time();
    socket_.async_send_to(boost::asio::buffer(&buff, sizeof(buff)), endpoint_,
      boost::bind(&udp_client::handle_send_to, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
  }
}

void udp_client::handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
{
  if (!error && bytes_sent > 0) {
    udp::endpoint sender_endpoint;
    std::cout<<"WYSLANE!!"<<std::endl;
    socket_.async_receive_from(
            boost::asio::buffer(reply, sizeof(reply)), sender_endpoint,
            boost::bind(&udp_client::handle_receive_from, this,
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred));
    timer_.expires_from_now(boost::posix_time::seconds(MAX_TIME_FOR_RESPONSE));
    timer_.async_wait(boost::bind(&udp_client::check_timer, this,
      boost::asio::placeholders::error));
  }
}

void udp_client::check_timer(const boost::system::error_code& /*e*/) {
  if (timer_.expires_at() <= boost::asio::deadline_timer::traits_type::now()) {
    std::cout << " \n\n\n   TIMEOUT     \n\n\n";
    for(uint i = 0; i < connections.size(); i++) {
      if (connections[i].ip.compare(host_) == 0)
          connections[i].credits--;
        if (connections[i].credits > 0)
          send();
        else
          connections[i].alive = 0;
    }
  }
}

void udp_client::handle_receive_from(const boost::system::error_code& error, size_t bytes_received) {
  if (!error && bytes_received > 0) {
    std::cout<<"HOST = " << host_ << std::endl;
    t2 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration msdiff = t2 - t1;
    std::cout << "Wysłanie pakietu UDP i odebranie odpowiedzi zajmuje " << msdiff.total_microseconds() << " mikrosekund" << std::endl;
    std::cerr << "Sent at: " << be64toh(reply[0]) << "\n" << "Answered at: " << be64toh(reply[1]) << "\n";
    for(uint i = 0; i < connections.size(); i++) {
      if (connections[i].ip.compare(host_) == 0) {
        connections[i].udp[connections[i].pos] = (int) msdiff.total_microseconds();
        connections[i].pos = (connections[i].pos+1)%10;
        connections[i].credits--;
      }
    }
  }
  sleep(interval_);
  send();
}
