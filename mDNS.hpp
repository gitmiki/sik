#ifndef mDNS_HPP
#define mDNS_HPP

#include "DNSpacket.hpp"

class mDNS
{
public:
  mDNS(boost::asio::io_service& io_service,
      const boost::asio::ip::address& listen_address,
      const boost::asio::ip::address& multicast_address,
      int interval
  );
  void prepare_PTR_query();
  void prepare_A_query();
  void prepare_PTR_response(unsigned char *response, uint16_t ID);
  void prepare_A_response();
  void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
  void handle_send_to(const boost::system::error_code& error);
  void send_query();
  void receive();
  void handle_timeout(const boost::system::error_code& error);
  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd, unsigned char answer[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)]);

private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::endpoint sender_endpoint_;
  boost::asio::deadline_timer timer_;
  unsigned char* my_name;
  int message_count_;
  std::string message_;
  unsigned char query_buf[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  int query_buf_length;
  unsigned char *query_name;
  int interval_;
};

#endif /* mDNS_HPP */
