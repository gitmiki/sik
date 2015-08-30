#ifndef mDNS_HPP
#define mDNS_HPP

#include "DNSpacket.hpp"

class mDNS
{
public:
  mDNS(boost::asio::io_service& io_service,
      const boost::asio::ip::address& listen_address,
      const boost::asio::ip::address& multicast_address,
      int interval,
      bool DNS_SD
  );
  std::string getIP();
  void prepare_PTR_query();
  void change_PTR_query_ID();
  void send_A_query(unsigned char* name);
  void response_PTR(uint16_t ID);
  void response_A(uint16_t ID);
  void ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host);
  void handle_send_to(const boost::system::error_code& error);
  void send_PTR_query();
  void receive();
  void handle_timeout(const boost::system::error_code& error);
  void handle_receive_from(const boost::system::error_code& error,
      size_t bytes_recvd);

private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::endpoint sender_endpoint_;
  boost::asio::deadline_timer timer_;
  unsigned char my_name[128];
  std::string my_ip;
  unsigned char response_PTR_buf[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  unsigned char response_A_buf[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  unsigned char answer[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  int message_count_;
  std::string message_;
  unsigned char query_PTR_buf[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  unsigned char query_A_buf[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  int query_buf_length;
  int interval_;
  bool DNS_SD;
};

#endif /* mDNS_HPP */
