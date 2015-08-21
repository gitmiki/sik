#ifndef UDP_SERVER_HPP
#define UDP_SERVER_HPP

#include "config.hpp"

using boost::asio::ip::udp;

class udp_server
{
public:
  udp_server(boost::asio::io_service& io_service);

private:
  void start_receive();

  void handle_receive(const boost::system::error_code& error,
      std::size_t /*bytes_transferred*/);

  void handle_send(boost::shared_ptr<std::string> /*message*/,
      const boost::system::error_code& /*error*/,
      std::size_t /*bytes_transferred*/);


  udp::socket socket_;
  udp::endpoint remote_endpoint_;
  boost::array<char, 1> recv_buffer_;
};

#endif /* UDP_SERVER_HPP */
