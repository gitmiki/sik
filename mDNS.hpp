#ifndef mDNS_HPP
#define mDNS_HPP

#include "DNSpacket.hpp"

const int MAX_BUFFER_SIZE = 256;

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
  void broadcast_SSH();
  void send_A_query(unsigned char* name);
  void response_PTR(uint16_t ID);
  void response_A(uint16_t ID);
  void format_to_DNS(unsigned char* dns, unsigned char* host);
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
  unsigned char ssh_service_name[128];
  std::string my_ip;
  unsigned char response_PTR_buf[sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery)];
  unsigned char response_A_buf[sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery)];
  unsigned char answer[sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery)];
  unsigned char ssh_broadcast_buffer[sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery)];
  int message_count_;
  std::string message_;
  unsigned char query_PTR_buf[sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery)];
  unsigned char query_A_buf[sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery)];
  int query_buf_length;
  int interval_;
  bool DNS_SD;
};

#endif /* mDNS_HPP */
