#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

using boost::asio::ip::tcp;

class tcp_client
{
public:
  tcp_client(boost::asio::io_service& io_service, int interval);
  void start_sending(const boost::system::error_code& /*e*/);
  void connect(const std::string& host);

private:
  tcp::resolver resolver_;
  tcp::socket socket_;
  //boost::asio::streambuf request_;
  //boost::asio::streambuf response_;
  int interval_;
  boost::asio::deadline_timer timer_;
};

#endif /* TCP_CLIENT_HPP */
