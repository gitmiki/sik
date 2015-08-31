#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <netinet/ip_icmp.h>
#include "connection.hpp"
#include "icmp_client.hpp"
#include "err.hpp"

#define BSIZE 1000
#define ICMP_HEADER_LEN 8

extern std::vector<Connection> connections;

typedef struct indeks {
  uint8_t i[3];
} indeks;

icmp_client::icmp_client(boost::asio::io_service& io_service, int interval)
  : io_service_(io_service),
    interval_(interval),
    timer_(io_service)
{
  timer_.expires_from_now(boost::posix_time::seconds(0));
  timer_.async_wait(boost::bind(&icmp_client::start_sending, this,
    boost::asio::placeholders::error));
}

void icmp_client::start_sending(const boost::system::error_code& /*e*/) {
  while (true) {
    for(uint i = 0; i < connections.size(); i++) {
      if ((connections[i].icmp_credits > 0) && (connections[i]._opoznienia)) {
        send(connections[i].ip);
        connections[i].icmp_credits--;
      }
    }
    sleep(interval_);
  }
}

void icmp_client::send(const std::string& host) {
  int sock;

  sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  if (sock < 0)
    syserr("socket");

  boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();
  send_ping_request(sock, (char*) host.c_str());
  while (!receive_ping_reply(sock))
    ;

  boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration msdiff = t2 - t1;
  for(uint i = 0; i < connections.size(); i++) {
    if (connections[i].ip.compare(host) == 0) {
      connections[i].icmp[connections[i].pos_icmp] = (int) msdiff.total_microseconds();
      connections[i].pos_icmp = (connections[i].pos_icmp+1)%10;
    }
  }

  if (close(sock) == -1)
    syserr("close");
}

void icmp_client::send_ping_request(int sock, char* s_send_addr) {
  struct addrinfo addr_hints;
  struct addrinfo *addr_result;
  struct sockaddr_in send_addr;

  struct icmp* icmp;

  char send_buffer[BSIZE+4];

  int err = 0;
  ssize_t data_len = 0;
  ssize_t icmp_len = 0;
  ssize_t len = 0;

  // 'converting' host/port in string to struct addrinfo
  memset(&addr_hints, 0, sizeof(struct addrinfo));
  addr_hints.ai_family = AF_INET;
  addr_hints.ai_socktype = SOCK_RAW;
  addr_hints.ai_protocol = IPPROTO_ICMP;
  err = getaddrinfo(s_send_addr, 0, &addr_hints, &addr_result);
  if (err != 0)
    syserr("getaddrinfo: %s\n", gai_strerror(err));

  send_addr.sin_family = AF_INET;
  send_addr.sin_addr.s_addr =
      ((struct sockaddr_in*) (addr_result->ai_addr))->sin_addr.s_addr;
  send_addr.sin_port = htons(0);
  freeaddrinfo(addr_result);

  memset(send_buffer, 0, sizeof(send_buffer));
  // initializing ICMP header

  indeks *ind;
  ind = (struct indeks*)&send_buffer;
  ind->i[0] = htons(33);
  ind->i[1] = htons(45);  //Nr indeksu na pierwszych trzech bajtach
  ind->i[2] = htons(43);

  icmp = (struct icmp *)&send_buffer[3];
  icmp->icmp_type = ICMP_ECHO;
  icmp->icmp_code = 0;
  icmp->icmp_id = 0x13; // process identified by PID
  icmp->icmp_seq = htons(0); // sequential number
  data_len = snprintf(((char*) send_buffer+ICMP_HEADER_LEN),
                      sizeof(send_buffer)-ICMP_HEADER_LEN, "BASIC PING!");
  if (data_len < 1)
    syserr("snprinf");
  icmp_len = data_len + ICMP_HEADER_LEN + 4; // packet is filled with 0
  icmp->icmp_cksum = 0; // checksum computed over whole ICMP package
  icmp->icmp_cksum = in_cksum((unsigned short*) icmp, icmp_len);

  send_buffer[icmp_len+3] = htons(7); //Nr grupy na ostatnim bajcie

  len = sendto(sock, (void*) icmp, icmp_len, 0, (struct sockaddr *) &send_addr,
               (socklen_t) sizeof(send_addr));
  if (icmp_len != (ssize_t) len)
    syserr("partial / failed write");

  //printf("wrote %zd bytes\n", len);
}

int icmp_client::receive_ping_reply(int sock) {
  struct sockaddr_in rcv_addr;
  socklen_t rcv_addr_len;

  struct ip* ip;
  struct icmp* icmp;

  char rcv_buffer[BSIZE];

  ssize_t ip_header_len = 0;
  //ssize_t data_len = 0;
  ssize_t icmp_len = 0;
  ssize_t len;


  memset(rcv_buffer, 0, sizeof(rcv_buffer));
  rcv_addr_len = (socklen_t) sizeof(rcv_addr);
  len = recvfrom(sock, (void*) rcv_buffer, sizeof(rcv_buffer), 0,
                 (struct sockaddr *) &rcv_addr, &rcv_addr_len);

  //printf("received %zd bytes from %s\n", len, inet_ntoa(rcv_addr.sin_addr));

  //recvfrom returns whole packet (with IP header)
  ip = (struct ip*) rcv_buffer;
  ip_header_len = ip->ip_hl << 2; // IP header len is in 4-byte words

  icmp = (struct icmp*) (rcv_buffer + ip_header_len); // ICMP header follows IP
  icmp_len = len - ip_header_len;

  if (icmp_len < ICMP_HEADER_LEN)
    fatal("icmp header len (%d) < ICMP_HEADER_LEN", icmp_len);

  if (icmp->icmp_type != ICMP_ECHOREPLY) {
    //printf("strange reply type (%d)\n", icmp->icmp_type);
    return 0;
  }

  //if (ntohs(icmp->icmp_id) != getpid())
  //  fatal("reply with id %d different from my pid %d", ntohs(icmp->icmp_id), getpid());

  //data_len = len - ip_header_len - ICMP_HEADER_LEN;
  //printf("correct ICMP echo reply; payload size %zd content %.*s\n", data_len,
  //       (int) data_len, (rcv_buffer+ip_header_len+ICMP_HEADER_LEN));
  return 1;
}

unsigned short icmp_client::in_cksum(unsigned short *addr, int len){
  int				nleft = len;
	int				sum = 0;
	unsigned short	*w = addr;
	unsigned short	answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

		/* 4mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(unsigned char *)(&answer) = *(unsigned char *)w ;
		sum += answer;
	}

		/* 4add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}
