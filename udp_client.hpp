#ifndef UDP_CLIENT_HPP
#define UDP_CLIENT_HPP

using boost::asio::ip::udp;

class udp_client
{
public:
  udp_client(boost::asio::io_service& io_service, const std::string& port, int interval);
  void handle_send_to(const boost::system::error_code& error, size_t bytes_sent, const std::string& host, boost::posix_time::ptime t1);
  void handle_receive_from(const boost::system::error_code& error, size_t bytes_received, const std::string& host, boost::posix_time::ptime t1);
  void start_sending(const boost::system::error_code& /*e*/);
  void send(const std::string& host);

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint endpoint_;
  boost::uint64_t answer[2];
  boost::uint64_t reply[2];
  int interval_;
  std::string port_;
  boost::asio::deadline_timer timer_;
};

#endif /* UDP_CLIENT_HPP */
