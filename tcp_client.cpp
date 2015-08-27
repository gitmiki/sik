#include "tcp_client.hpp"

tcp_client::tcp_client(boost::asio::io_service& io_service,
    const std::string& server)
  : resolver_(io_service),
    socket_(io_service)
{
  tcp::resolver::query query(server, "22");
  tcp::resolver::iterator endpoint_iterator = resolver_.resolve(query);
  tcp::resolver::iterator end;
  boost::system::error_code error = boost::asio::error::host_not_found;
  t1 = boost::posix_time::microsec_clock::local_time();
  while (error && endpoint_iterator != end) {
    socket_.close();
    socket_.connect(*endpoint_iterator++, error);
  }
  t2 = boost::posix_time::microsec_clock::local_time();
  socket_.close();
  boost::posix_time::time_duration msdiff = t2 - t1;
  std::cout << "Połączenie się za pomocą TCP z " << server << " zajmuje " << msdiff.total_microseconds() << " mikrosekund" << std::endl;
  std::cout << "Connected!" << std::endl;
}