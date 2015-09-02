#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include "boost/bind.hpp"

#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const short MULTICAST_PORT = 5353;
const char* SERVICE_NAME = "_opoznienia._udp.local.";
const char* SSH_SERVICE = "_ssh._tcp.local.";

#define CREDITS_RATE 12;

#include "mDNS.hpp"
#include "connection.hpp"
#include "udp_client.hpp"

extern std::vector<Connection> connections;

mDNS::mDNS(boost::asio::io_service& io_service,
    const boost::asio::ip::address& listen_address,
    const boost::asio::ip::address& multicast_address,
    int interval,
    bool DNS_SD
  )
  : socket_(io_service),
    endpoint_(multicast_address, 5353),
    timer_(io_service),
    message_count_(0),
    interval_(interval),
    DNS_SD(DNS_SD)
{
  // Create the socket so that multiple may be bound to the same address.
  boost::asio::ip::udp::endpoint listen_endpoint(
      listen_address, MULTICAST_PORT);
  socket_.open(listen_endpoint.protocol());
  socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
  socket_.bind(listen_endpoint);

  // Join the multicast group.
  socket_.set_option(
      boost::asio::ip::multicast::join_group(multicast_address));

  char hostname[64];
  gethostname(hostname, 64);
  //unsigned char tmp[strlen(hostname)+strlen((char*) SERVICE_NAME)+1];

  unsigned int i = 0;
  for (uint i = 0; i < 128; i++) {
    my_name[i] = '\0';
    ssh_service_name[i] = '\0';
  }
  for (; i < strlen(hostname); i++) {
    my_name[i] = hostname[i];
    ssh_service_name[i] = hostname[i];
  }
  uint j = i;
  my_name[i++] = '.';
  ssh_service_name[j++] = '.';

  for (; i <= strlen(hostname) + strlen((char*) SERVICE_NAME); i++) {
    my_name[i] = SERVICE_NAME[i-strlen(hostname)-1];
  }
  for (; j <= strlen(hostname) + strlen((char*) SSH_SERVICE); j++) {
    ssh_service_name[j] = SSH_SERVICE[j-strlen(hostname)-1];
  }

  //std::cout << "MY_NAME = " << my_name << std::endl;

  //std::cout << "SSH_SERVICE_NAME = " << ssh_service_name << std::endl;


  my_ip = getIP();
  srand ( time(NULL) );
  prepare_PTR_query();
  send_PTR_query();
  if (DNS_SD) {
    broadcast_SSH();
  }

  receive();
}

std::string mDNS::getIP()
{
  struct ifaddrs *myaddrs, *ifa;
  void *in_addr;
  char buf[64];

  if(getifaddrs(&myaddrs) != 0)
  {
      perror("getifaddrs");
      exit(1);
  }

  // ALTERNATYWNA WERSJA DO LABÓW ZAMIAST CAŁEGO PONIŻSZEGO IFa

  /*
  ifa = myaddrs;
  ifa = ifa->ifa_next->ifa_next->ifa_next->ifa_next;
  struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
  in_addr = &s4->sin_addr;
  if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
  {
      printf("%s: inet_ntop failed!\n", ifa->ifa_name);
  }
  */

  for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
  {
    if (ifa->ifa_addr == NULL)
      continue;
    if (!(ifa->ifa_flags & IFF_UP))
      continue;

    switch (ifa->ifa_addr->sa_family)
    {
      case AF_INET:
      {
        struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
        in_addr = &s4->sin_addr;
        break;
      }
      default:
        continue;
      }

      if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
      {
          printf("%s: inet_ntop failed!\n", ifa->ifa_name);
      }
  }
  freeifaddrs(myaddrs);
  return std::string(buf);
}

void mDNS::format_to_DNS(unsigned char* dns, unsigned char* host) {
    unsigned int lock = 0;
    for(unsigned int i = 0 ; i < strlen((char*)host) ; i++) {
        if(host[i]=='.') {
            *dns++ = i-lock;
            for(;lock<i;lock++)
                *dns++=host[lock];
            lock++;
        }
    }
    *dns++='\0';
}

void mDNS::handle_send_to(const boost::system::error_code& error)
{
  //std::cout << "WYSŁANE!!!\n";
  if (!error)
  {
    timer_.expires_from_now(boost::posix_time::seconds(interval_));
    timer_.async_wait(
        boost::bind(&mDNS::handle_timeout, this,
          boost::asio::placeholders::error));
  }
}

void mDNS::handle_timeout(const boost::system::error_code& error)
{
  if (!error)
  {
    change_PTR_query_ID();
    send_PTR_query();
    if (DNS_SD) {
      broadcast_SSH();
    }
  }
}

void mDNS::broadcast_SSH() {
  for (unsigned int i = 0; i < sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery); i++) // czyścimy buffer
    ssh_broadcast_buffer[i] = '\0';
  int length = 0;
  DNSHeader *header = NULL;
  RRecord *record = NULL;
  header = (DNSHeader*)&ssh_broadcast_buffer;
  length += sizeof(DNSHeader);
  header->ID = 0;
  header->rd = 1;
  header->qr = 1;
  header->tc = 0;
  header->aa = 1;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 1;
  header->z = 0;
  header->ra = 0;
  header->qdcount = 0;
  header->ancount = htons(1);
  header->nscount = 0;
  header->arcount = 0;
  unsigned char* record_name = (unsigned char*)&ssh_broadcast_buffer[length];
  format_to_DNS(record_name, ssh_service_name);
  length += strlen((const char*) record_name) + 1;
  record = (RRecord*)&ssh_broadcast_buffer[length];
  record->rtype = htons(1);
  record->rclass = htons(1);
  record->ttl = htonl(10);
  record->rdlength = htons(sizeof(uint32_t));//htons(strlen((const char*) record_rdata));
  length += sizeof(RRecord) - sizeof(uint16_t);
  std::vector<std::string> strs;
  boost::split(strs, my_ip, boost::is_any_of("."));
  IPRecord *ip = NULL;
  ip = (IPRecord*)&ssh_broadcast_buffer[length];
  for (unsigned int i = 0; (i < strs.size()) or (i < 4); i++)
    ip->field[i] = (uint8_t) atoi(strs[i].c_str());
  length += sizeof(IPRecord);
  socket_.send_to(
      boost::asio::buffer(ssh_broadcast_buffer, length), endpoint_
      );
}

void mDNS::prepare_PTR_query() {
  for (unsigned int i = 0; i < sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery); i++) // czyścimy buffer
    query_PTR_buf[i] = '\0';
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  int length = 0;
  header = (DNSHeader*)&query_PTR_buf;
  length += sizeof(DNSHeader);
  header->ID = htons (rand()%65537);
  header->rd = 1;
  header->qr = 0;
  header->tc = 0;
  header->aa = 0;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 0;
  header->z = 0;
  header->ra = 0;
  if (DNS_SD)
    header->qdcount = htons(2);
  else
    header->qdcount = htons(1);
  header->ancount = 0;
  header->nscount = 0;
  header->arcount = 0;
  unsigned char *query_name = (unsigned char*)&query_PTR_buf[length];
  unsigned char *DNSname = (unsigned char*) SERVICE_NAME;
  format_to_DNS(query_name, DNSname);
  length += strlen((const char*) query_name) + 1;
  query = (DNSQuery*)&query_PTR_buf[length];
  length += sizeof(DNSQuery);
  query->type = htons(12); // PTR
  query->qclass = htons(1);
  /*if (DNS_SD) {
    DNSQuery *ssh_query;
    unsigned char *ssh_query_name = (unsigned char*)&query_PTR_buf[length];
    unsigned char *ssh_DNSname = (unsigned char*) SSH_QUERY;
    format_to_DNS(ssh_query_name, ssh_DNSname);
    length += strlen((const char*) ssh_query_name) + 1;
    ssh_query = (DNSQuery*)&query_PTR_buf[length];
    length += sizeof(DNSQuery);
    ssh_query->type = htons(1); //A
    ssh_query->qclass = htons(1);
  }*/
  query_buf_length = length;
}

void mDNS::change_PTR_query_ID() {
  DNSHeader *header = NULL;
  header = (DNSHeader*)&query_PTR_buf;
  header->ID = htons (rand()%65537);;
}

void mDNS::send_A_query(unsigned char* name) {
  for (unsigned int i = 0; i < sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery); i++) // czyścimy buffer
    query_A_buf[i] = '\0';
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  header = (DNSHeader*)&query_A_buf;
  header->ID = htons(rand()%65537);
  header->rd = 1;
  header->qr = 0;
  header->tc = 0;
  header->aa = 0;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 0;
  header->z = 0;
  header->ra = 0;
  header->qdcount = htons(1);
  header->ancount = 0;
  header->nscount = 0;
  header->arcount = 0;
  unsigned char *query_name = (unsigned char*)&query_A_buf[sizeof(DNSHeader)];
  //unsigned char *DNSname = (unsigned char*) SERVICE_NAME;
  format_to_DNS(query_name, name);
  query = (DNSQuery*)&query_A_buf[sizeof(DNSHeader) + strlen((const char*) query_name)+1];
  query->type = htons(1); // A
  query->qclass = htons(1);
  query_buf_length = sizeof(DNSHeader) + strlen((const char*)query_name) + 1 + sizeof(DNSQuery);
  socket_.send_to(
      boost::asio::buffer(query_A_buf, sizeof(DNSHeader) + strlen((const char*) query_name) + 1 + sizeof(DNSQuery)), endpoint_
      );
}

void mDNS::response_PTR(uint16_t ID) {
  for (unsigned int i = 0; i < sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery); i++) // czyścimy buffer
    response_PTR_buf[i] = '\0';
  int length = 0;
  DNSHeader *header = NULL;
  RRecord *record = NULL;
  header = (DNSHeader*)&response_PTR_buf;
  length += sizeof(DNSHeader);
  header->ID = htons (ID);
  header->rd = 1;
  header->qr = 1;
  header->tc = 0;
  header->aa = 1;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 1;
  header->z = 0;
  header->ra = 0;
  header->qdcount = 0;
  header->ancount = htons(1);
  header->nscount = 0;
  header->arcount = 0;
  unsigned char* record_name = (unsigned char*)&response_PTR_buf[length];
  format_to_DNS(record_name, my_name);
  length += strlen((const char*) record_name) + 1;
  record = (RRecord*)&response_PTR_buf[length];
  record->rtype = htons(12);
  record->rclass = htons(1);
  record->ttl = htons(10);
  length += sizeof(RRecord) - sizeof(uint16_t);
  unsigned char* record_rdata = (unsigned char*)&response_PTR_buf[length];
  format_to_DNS(record_rdata, my_name);
  length += strlen((const char*) record_rdata) + 1;

  record->rdlength = htons(strlen((const char*) record_rdata));
  socket_.send_to(
      boost::asio::buffer(response_PTR_buf, length), endpoint_
      );
}

void mDNS::response_A(uint16_t ID) {
  for (unsigned int i = 0; i < sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery); i++) // czyścimy buffer
    response_A_buf[i] = '\0';
  int length = 0;
  DNSHeader *header = NULL;
  RRecord *record = NULL;
  header = (DNSHeader*)&response_A_buf;
  length += sizeof(DNSHeader);
  header->ID = htons(ID);
  header->rd = 1;
  header->qr = 1;
  header->tc = 0;
  header->aa = 1;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 1;
  header->z = 0;
  header->ra = 0;
  header->qdcount = 0;
  header->ancount = htons(1);
  header->nscount = 0;
  header->arcount = 0;
  unsigned char* record_name = (unsigned char*)&response_A_buf[length];
  format_to_DNS(record_name, my_name);
  length += strlen((const char*) record_name) + 1;
  record = (RRecord*)&response_A_buf[length];
  record->rtype = htons(1);
  record->rclass = htons(1);
  record->ttl = htonl(10);
  record->rdlength = htons(sizeof(uint32_t));//htons(strlen((const char*) record_rdata));
  length += sizeof(RRecord) - sizeof(uint16_t);
  std::vector<std::string> strs;
  boost::split(strs, my_ip, boost::is_any_of("."));
  IPRecord *ip = NULL;
  ip = (IPRecord*)&response_A_buf[length];
  for (unsigned int i = 0; (i < strs.size()) or (i < 4); i++)
    ip->field[i] = (uint8_t) atoi(strs[i].c_str());
  length += sizeof(IPRecord);
  socket_.send_to(
      boost::asio::buffer(response_A_buf, length), endpoint_
      );
}

void mDNS::send_PTR_query() {
  socket_.async_send_to(
      boost::asio::buffer(query_PTR_buf, query_buf_length), endpoint_,
      boost::bind(&mDNS::handle_send_to, this,
        boost::asio::placeholders::error));
}

void mDNS::receive() {
  for (unsigned int i = 0; i < sizeof(DNSHeader) + MAX_BUFFER_SIZE + sizeof(DNSQuery); i++) // czyścimy buffer
    answer[i] = '\0';
  socket_.async_receive(
    boost::asio::buffer(answer),// sender_endpoint_,
    boost::bind(&mDNS::handle_receive_from, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred));
}

void mDNS::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd)
{
  //receive();

  if (!error && bytes_recvd > 0)
  {
    DNSHeader *header = NULL;
    header = (DNSHeader*)answer;
    Connection c;
    int length = sizeof(DNSHeader);
    if (ntohs(header->tc) == 0) { // przyjmujemy tylko nie truncated
      if (ntohs(header->qr) == 0) {// query
          for (int i = 0; i < ntohs(header->qdcount); i++) {
            DNSQuery *query = NULL;
            unsigned char* query_;
            query_ = (unsigned char*)&answer[length];
            length += strlen((const char*) query_) + 1;
            query = (DNSQuery*)&answer[length];
            length += sizeof(DNSQuery);
            char translation[strlen((char*) query_)];
            for (unsigned int i = 1; i < strlen((char*) query_); i++) {
              int x = (int) query_[i];
              if (x >= 32 && x <= 126) // check if printable
                translation[i-1] = static_cast<char>(x);//(char) std::to_string(x).c_str();
              else
                translation[i-1] = '.';
            }
            translation[strlen((char*) query_)-1] = '.';
            translation[strlen((char*) query_)] = '\0';
            //std::cout << "OTRZYMANE PYTANIE TO " << translation << std::endl;
            switch (ntohs(query->type)) {
              case 12: //PTR
                if (strcmp(translation, SERVICE_NAME) == 0) {
                  //std::cout << " Otrzymano zapytanie PTR \n";
                  response_PTR(ntohs(header->ID));
                }
                break;
              case 1: //A
                if (strcmp(translation, (const char*) my_name) == 0) {
                  //std::cout << " Otrzymano zapytanie A\n";
                  response_A(ntohs(header->ID));
                }
                break;
              default: // ignorujemy
                //std::cout << "cicho ignorujemy zapytanie\n";
                break;
            }
          }
      }
      else { // response
        // Omijamy pytania jeśli są
        for (int i = 0; i < ntohs(header->qdcount); i++) {
          unsigned char* query_;
          query_ = (unsigned char*)&answer[length];
          length += strlen((const char*) query_) + 1 + sizeof(DNSQuery);
        }

        for (int i = 0; i < ntohs(header->ancount); i++) {
          RRecord *record = NULL;
          unsigned char* domain_name;
          domain_name = (unsigned char*)&answer[length];
          length += strlen((const char*) domain_name) + 1;
          record = (RRecord*)&answer[length];
          length += sizeof(RRecord)-sizeof(uint16_t);
          unsigned char* response;
          response = (unsigned char*)&answer[length];
          length += strlen((const char*) response) + 1;
          char translation[strlen((char*) response)];
          char domain[strlen((char*) domain_name)];
          for (unsigned int i = 1; i < strlen((char*) domain_name); i++) {
            int x = (int) domain_name[i];
            if (x >= 32 && x <= 126) // check if printable
              domain[i-1] = static_cast<char>(x);
            else
              domain[i-1] = '.';
          }
          domain[strlen((char*) domain_name)-1] = '.';
          domain[strlen((char*) domain_name)] = '\0';
          std::string IP = "";
          std::ostringstream convert;
          uint j = 0; bool pass = true;
          switch (ntohs(record->rtype)) {
            case 12: //PTR
              //std::cout << " Otrzymano odpowiedź PTR \n";
              for (unsigned int i = 1; i < strlen((char*) response); i++) {
                int x = (int) response[i];
                if (x >= 32 && x <= 126) // check if printable
                  translation[i-1] = static_cast<char>(x);
                else
                  translation[i-1] = '.';
              }
              translation[strlen((char*) response)-1] = '.';
              translation[strlen((char*) response)] = '\0';
              //std::cout << "OTRZYMANA ODPOWIEDŹ TO " << translation << std::endl;

              send_A_query((unsigned char*) translation);
              break;
            case 1: //A
              //std::cout << " Otrzymano odpowiedź A\n";
              for (uint i = 0; i < (strlen((char*) response)); i++) {
                convert << (int) response[i];
                //std::cout << " i = " << i << std::endl;
                //if (i != (strlen((char*) response) - 1))
                //  convert << '.';
              }
              convert << '.';
              response = (unsigned char*)&answer[length];
              length += strlen((const char*) response) + 1;
              for (uint i = 0; i < (stlen((char*) response)); i++) {
                convert << (int) response[i];
              }
              

              IP = convert.str();
              std::cout << "Otrzymane IP to " << IP << std::endl;
              j = 0;
              pass = true;
              while ((domain[j] != '.') and (j < strlen((char*) domain)))
                j++;
              pass = true;
              for (uint i = 0; i < strlen((char*) SERVICE_NAME); i++) {
                pass = (i+j+1 < strlen((char*) domain));
                if (!pass)
                  break;
                pass = (domain[i+j+1] == SERVICE_NAME[i]);
                if (!pass)
                  break;
              }
              if (pass) {
                uint i;
                for(i = 0; i < connections.size(); i++) {
                  if (connections[i].ip.compare(IP) == 0) {
                    connections[i].udp_credits = CREDITS_RATE;
                    connections[i].icmp_credits = CREDITS_RATE;
                    connections[i]._opoznienia = true;
                    break;
                  }
                }
                if (i == connections.size()) {
                  c.ip = IP;
                  c.udp_credits = CREDITS_RATE;
                  c.tcp_credits = 0;
                  c.icmp_credits = CREDITS_RATE;
                  c._opoznienia = true;
                  c._ssh = false;
                  c.pos_udp = 0;
                  c.pos_tcp = 0;
                  c.pos_icmp = 0;
                  for (int i = 0; i < 10; i++) {
                    c.udp[i] = 0; c.ssh[i] = 0; c.icmp[i] = 0;
                  }
                  connections.push_back(c);
                }
              }
              else {
                pass = true;
                for (uint i = 0; i < strlen((char*) SSH_SERVICE); i++) {
                  pass = (i+j+1 < strlen((char*) domain));
                  if (!pass)
                    break;
                  pass = (domain[i+j+1] == SSH_SERVICE[i]);
                  if (!pass)
                    break;
                }
                if (pass) {
                  uint i;
                  for(i = 0; i < connections.size(); i++) {
                    if (connections[i].ip.compare(IP) == 0) {
                      connections[i].tcp_credits = CREDITS_RATE;
                      connections[i]._ssh = true;
                      break;
                    }
                  }
                  if (i == connections.size()) {
                    c.ip = IP;
                    c.udp_credits = 0;
                    c.tcp_credits = CREDITS_RATE;
                    c.icmp_credits = 0;
                    c._opoznienia = false;
                    c._ssh = true;
                    c.pos_udp = 0;
                    c.pos_tcp = 0;
                    c.pos_icmp = 0;
                    for (int i = 0; i < 10; i++) {
                      c.udp[i] = 0; c.ssh[i] = 0; c.icmp[i] = 0;
                    }
                    connections.push_back(c);
                  }
                }
              }

              break;
            default: // ignorujemy
              break;
          }
        }
      }
    }
  }
  receive();
}
