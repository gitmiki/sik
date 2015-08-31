#ifndef ICMP_CLIENT_HPP
#define ICMP_CLIENT_HPP

class icmp_client
{
public:
  icmp_client(boost::asio::io_service& io_service, int interval);
  unsigned short in_cksum(unsigned short *addr, int len);
  void start_sending(const boost::system::error_code& /*e*/);
  void send(const std::string& host);
  void send_ping_request(int sock, char* s_send_addr);
  int receive_ping_reply(int sock);

private:
  boost::asio::io_service& io_service_;
  int interval_;
  boost::asio::deadline_timer timer_;
};

#endif /* ICMP_CLIENT_HPP */
