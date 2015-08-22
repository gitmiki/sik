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
  udp_client(boost::asio::io_service& io_service, const std::string& host, const std::string& port)
    : io_service_(io_service),
      socket_(io_service, udp::endpoint(udp::v4(), 0))
  {
    udp::resolver resolver(io_service_);
    udp::resolver::query query(udp::v4(), host, port);
    udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;
    struct timeval tv;
	  gettimeofday(&tv,NULL);
    t1 = boost::posix_time::microsec_clock::local_time();

	  boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
    boost::uint64_t buff = htobe64(b);
    size_t request_length = sizeof(buff);

    socket_.async_send_to(boost::asio::buffer(&buff, request_length), endpoint_,
      boost::bind(&udp_client::handle_send_to, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

  }

  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent)
  {
    boost::uint64_t reply[2];
    udp::endpoint sender_endpoint;
    size_t reply_length = socket_.receive_from(
        boost::asio::buffer(reply, max_length), sender_endpoint);

    t2 = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration msdiff = t2 - t1;
    std::cout << "Wysłanie pakietu UDP i odebranie odpowiedzi zajmuje " << msdiff.total_microseconds() << " mikrosekund" << std::endl;
    std::cerr << "Sent at: " << be64toh(reply[0]) << "\n" << "Answered at: " << be64toh(reply[1]) << "\n";
  }

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint endpoint_;
  boost::posix_time::ptime t1;
  boost::posix_time::ptime t2;
  enum { max_length = 1024 };
  //boost::uint64_t answer[0];
  boost::uint64_t answer[2];
  //char [max_length];
};
