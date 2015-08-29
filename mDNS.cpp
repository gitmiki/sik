#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <boost/asio.hpp>
#include "boost/bind.hpp"

const short MULTICAST_PORT = 5353;
const char* SERVICE_NAME = "_opoznienia._udp.local.";

#include "mDNS.hpp"

mDNS::mDNS(boost::asio::io_service& io_service,
    const boost::asio::ip::address& listen_address,
    const boost::asio::ip::address& multicast_address,
    int interval
  )
  : socket_(io_service),
    endpoint_(multicast_address, 5353),
    timer_(io_service),
    message_count_(0),
    interval_(interval)
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
  for (; i < strlen(hostname); i++) {
    my_name[i] = hostname[i];
  }
  my_name[i++] = '.';
  for (; i <= strlen(hostname) + strlen((char*) SERVICE_NAME); i++) {
    my_name[i] = SERVICE_NAME[i-strlen(hostname)-1];
  }
  my_name[i] = '\0';
  //std::cout<< "my_name = " << my_name << std::endl;
  //prepare_PTR_query();
  srand ( time(NULL) );
  prepare_PTR_query();
  send_query();

  receive();
}

void mDNS::ChangetoDnsNameFormat(unsigned char* dns, unsigned char* host) {
    unsigned int lock = 0;
    for(unsigned int i = 0 ; i < strlen((char*)host) ; i++)
    {
        if(host[i]=='.')
        {
            *dns++ = i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host[lock];
            }
            lock++;
        }
    }
    *dns++='\0';
}


void mDNS::handle_send_to(const boost::system::error_code& error)
{
  std::cout << "WYSŁANE!!!\n";
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
    send_query();
  }
}

void mDNS::prepare_PTR_query() {
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  header = (DNSHeader*)&query_buf;
  header->ID = htons (rand()%65537);
  std::cout << "wylosowane ID to " << ntohs(header->ID) << std::endl;
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
  unsigned char *query_name = (unsigned char*)&query_buf[sizeof(DNSHeader)];
  unsigned char *DNSname = (unsigned char*) SERVICE_NAME;
  ChangetoDnsNameFormat(query_name, DNSname);
  query = (DNSQuery*)&query_buf[sizeof(DNSHeader) + strlen((const char*) query_name)+1];
  query->type = htons(12); // PTR
  query->qclass = htons(1);
  query_buf_length = sizeof(DNSHeader) + strlen((const char*)query_name) + 1 + sizeof(DNSQuery);
  std::cout << "Wartość header->qr przed wysłaniem to " << ntohl(header->qr) << std::endl;
}

void mDNS::change_PTR_query_ID() {
  DNSHeader *header = NULL;
  header = (DNSHeader*)&query_buf;
  header->ID = htons (rand()%65537);
  std::cout << "wylosowane ID to " << ntohs(header->ID) << std::endl;
}

void mDNS::prepare_A_query() {

}

void mDNS::response_PTR(uint16_t ID) {
  unsigned char response[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  DNSHeader *header = NULL;
  RRecord *record = NULL;
  header = (DNSHeader*)&response;
  header->ID = htons (ID);
  header->rd = 1;
  header->qr = 1;
  header->tc = 0;
  header->aa = 1;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 0;
  header->z = 0;
  header->ra = 0;
  header->qdcount = htons(1);
  header->ancount = htons(1);
  header->nscount = 0;
  header->arcount = 0;
  //query_name = (unsigned char*)&query_buf[sizeof(DNSHeader)];
  //unsigned char* tmp = (unsigned char*) my_name;
  //std::cout<< "my_name = " << my_name << std::endl;
  unsigned char* record_name = (unsigned char*)&response[sizeof(DNSHeader)];
  //unsigned char *DNSname = (unsigned char*) my_name;
  ChangetoDnsNameFormat(record_name, my_name);

  char translation[strlen((char*) record_name)];
  for (unsigned int i = 1; i < strlen((char*) record_name); i++) {
    int x = (int) record_name[i];
    //std::cout<< x << " ";
    if (x >= 32 && x <= 126) // check if printable
      translation[i-1] = static_cast<char>(x);//(char) std::to_string(x).c_str();
    else
      translation[i-1] = '.';
  }
  translation[strlen((char*) record_name)-1] = '.';
  std::cout << "Wysyłana odpowiedź to " << translation << std::endl;


  record = (RRecord*)&response[sizeof(DNSHeader) + strlen((const char*) record_name)+1];
  record->rtype = htons(12);
  record->rclass = htons(1);
  record->ttl = htons(10);
  record->rdlength = htons(strlen((const char*) record_name));
  unsigned char* record_rdata = (unsigned char*)&response[sizeof(DNSHeader) + strlen((const char*) record_name)+1 + sizeof(RRecord)];
  //unsigned char *DNSname = (unsigned char*) my_name;
  ChangetoDnsNameFormat(record_rdata, my_name);

  socket_.send_to(
      boost::asio::buffer(response, sizeof(DNSHeader) + strlen((const char*) record_name) + 1 + sizeof(RRecord) + strlen((const char*) record_rdata + 1)), endpoint_
      );
  //std::cout << "my dns name = " << record_name << std::endl;
  //unsigned char* DNSname = (unsigned char*) SERVICE_NAME;
  //ChangetoDnsNameFormat(query_name, DNSname);
  //query = (DNSQuery*)&query_buf[sizeof(DNSHeader) + strlen((const char*) query_name)+1];
  //query->type = htons(12); // PTR
  //query->qclass = htons(1);
  //query_buf_length = sizeof(DNSHeader) + strlen((const char*)query_name) + 1 + sizeof(DNSQuery);
}

void mDNS::prepare_A_response() {

}

void mDNS::send_query() {
  socket_.async_send_to(
      boost::asio::buffer(query_buf, query_buf_length), endpoint_,
      boost::bind(&mDNS::handle_send_to, this,
        boost::asio::placeholders::error));
}

void mDNS::receive() {
  unsigned char answer[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  //std::cout<< "my_name = " << my_name << std::endl;
  socket_.async_receive(
    boost::asio::buffer(answer),// sender_endpoint_,
    boost::bind(&mDNS::handle_receive_from, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred,
      answer));
}

void mDNS::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd, unsigned char answer[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)])
{
  receive();

  if (!error && bytes_recvd > 0)
  {
    DNSHeader *header = NULL;
    header = (DNSHeader*)answer;
    std::cout<<"Otrzymane ID to " << ntohs(header->ID) << std::endl;
    std::cout<<"Pytanie/Odpowiedz = " << ntohs(header->qr) << std::endl;


    //std::cout<< "Wartość header->qr to " << ntohs(header->qr) << std::endl


    if (ntohs(header->tc) == 0) { // przyjmujemy tylko nie truncated
      if (ntohs(header->qr) == 0) {// query
        DNSQuery *query = NULL;
        unsigned char* query_;
        query_ = (unsigned char*)&answer[sizeof(DNSHeader)];
        query = (DNSQuery*)&answer[sizeof(DNSHeader) + strlen((const char*) query_) + 1];
        char translation[strlen((char*) query_)];
        for (unsigned int i = 1; i < strlen((char*) query_); i++) {
          int x = (int) query_[i];
          //std::cout<< x << " ";
          if (x >= 32 && x <= 126) // check if printable
            translation[i-1] = static_cast<char>(x);//(char) std::to_string(x).c_str();
          else
            translation[i-1] = '.';
        }
        translation[strlen((char*) query_)-1] = '.';
        std::cout << "OTRZYMANE PYTANIE TO " << translation << std::endl;
        switch (ntohs(query->type)) {
          case 12: //PTR
            if (strcmp(translation, SERVICE_NAME) == 0) {
              std::cout << " Otrzymano zapytanie PTR \n";
              response_PTR(ntohs(header->ID));
            }
            break;
          case 1: //A
            std::cout << " Otrzymano zapytanie A\n";
            break;
          default: // ignorujemy
            std::cout << "cicho ignorujemy zapytanie\n";
            break;
        }

      }
      else { // response
        std::cout << "ROZPOYCZNAMY CZYTANIE ODPOWIEDZI!!!!\n";
        RRecord *record = NULL;
        unsigned char* response;
        response = (unsigned char*)&answer[sizeof(DNSHeader)];
        record = (RRecord*)&answer[sizeof(DNSHeader) + strlen((const char*) response) + 1];
        char translation[strlen((char*) response)];
        for (unsigned int i = 1; i < strlen((char*) response); i++) {
          int x = (int) response[i];
          //std::cout<< x << " ";
          if (x >= 32 && x <= 126) // check if printable
            translation[i-1] = static_cast<char>(x);//(char) std::to_string(x).c_str();
          else
            translation[i-1] = '.';
        }
        translation[strlen((char*) response)-1] = '.';
        std::cout << "OTRZYMANA ODPOWIEDŹ TO " << translation << std::endl;

        switch (ntohs(record->rtype)) {
          case 12: //PTR
            std::cout << " Otrzymano odpowiedź PTR \n";
            break;
          case 1: //A
            std::cout << " Otrzymano odpowiedź A\n";
            break;
          default: // ignorujemy
            std::cout << "cicho ignorujemy odpowiedź\n";
            break;
        }
      }
    }
  }
  //receive();
}
