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

udp_client::udp_client(boost::asio::io_service& io_service, const std::string& port, int interval)
  : io_service_(io_service),
    socket_(io_service, udp::endpoint(udp::v4(), 0)),
    interval_(interval),
    port_(port),
    timer_(io_service)
{
  timer_.expires_from_now(boost::posix_time::seconds(0));
  timer_.async_wait(boost::bind(&udp_client::start_sending, this,
    boost::asio::placeholders::error));
}

void udp_client::start_sending(const boost::system::error_code& /*e*/) {
  while (true) {
    for(uint i = 0; i < connections.size(); i++) {
      if ((connections[i].udp_credits > 0) && (connections[i]._opoznienia)) {
        send(connections[i].ip);
        connections[i].udp_credits--;
      }
    }
    sleep(interval_);
  }
}

void udp_client::send(const std::string& host) {
  udp::resolver resolver(io_service_);
  udp::resolver::query query(udp::v4(), host, port_);
  udp::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
  boost::uint64_t buff = htobe64(b);
  boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
  socket_.async_send_to(boost::asio::buffer(&buff, sizeof(buff)), endpoint_,
    boost::bind(&udp_client::handle_send_to, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred,
      host, t1));
}

void udp_client::handle_send_to(const boost::system::error_code& error, size_t bytes_sent, const std::string& host, boost::posix_time::ptime t1) {
  if (!error && bytes_sent > 0) {
    udp::endpoint sender_endpoint;
    //std::cout<<"WYSLANE!!"<<std::endl;
    socket_.async_receive_from(
            boost::asio::buffer(reply, sizeof(reply)), sender_endpoint,
            boost::bind(&udp_client::handle_receive_from, this,
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred,
              host, t1));
  }
}

void udp_client::handle_receive_from(const boost::system::error_code& error, size_t bytes_received, const std::string& host, boost::posix_time::ptime t1) {
  if (!error && bytes_received > 0) {
    //std::cout<<"HOST = " << host << std::endl;
    boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration msdiff = t2 - t1;
    //std::cout << "Wysłanie pakietu UDP i odebranie odpowiedzi zajmuje " << msdiff.total_microseconds() << " mikrosekund" << std::endl;
    //std::cerr << "Sent at: " << be64toh(reply[0]) << "\n" << "Answered at: " << be64toh(reply[1]) << "\n";
    for(uint i = 0; i < connections.size(); i++) {
      if (connections[i].ip.compare(host) == 0) {
        connections[i].udp[connections[i].pos_udp] = (int) msdiff.total_microseconds();
        connections[i].pos_udp = (connections[i].pos_udp+1)%10;
      }
    }
  }
  sleep(interval_);
  //send();
}
