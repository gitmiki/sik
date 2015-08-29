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
  //for (unsigned int i = 0; i < strlen(hostname); i++) {
  //  my_name[i] = hostname[i];
  //}
  //service_name = (unsigned char*) SERVICE_NAME;// + (unsigned char*) hostname;
  //my_name += (unsigned char*) hostname;

  //std::cout<< "my_name = " << service_name << "\n Długość nazwy hosta = " << strlen(hostname) << std::endl;
  unsigned char tmp[strlen(hostname)+strlen((char*) SERVICE_NAME)+1];
  unsigned int i = 0;
  for (; i < strlen(hostname); i++) {
    tmp[i] = hostname[i];
    std::cout << tmp[i] << std::endl;
  }
  tmp[i++] = '.';
  for (; i <= strlen(hostname) + strlen((char*) SERVICE_NAME); i++) {
    tmp[i] = SERVICE_NAME[i-strlen(hostname)-1];
  }
  my_name = (unsigned char*) tmp;
  //std::cout<< "my_name = " << my_name << std::endl;
  //prepare_PTR_query();
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
            lock++; //or lock=i+1;
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
    prepare_PTR_query();
    send_query();
  }
}

void mDNS::prepare_PTR_query() {
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  header = (DNSHeader*)&query_buf;
  header->ID = htons (rand()%65537);
  std::cout << "wylosowane ID to " << ntohs(header->ID) << std::endl;
  header->rd = 0;
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
  query_name = (unsigned char*)&query_buf[sizeof(DNSHeader)];
  unsigned char* DNSname = (unsigned char*) SERVICE_NAME;
  ChangetoDnsNameFormat(query_name, DNSname);
  query = (DNSQuery*)&query_buf[sizeof(DNSHeader) + strlen((const char*) query_name)+1];
  query->type = htons(12); // PTR
  query->qclass = htons(1);
  query_buf_length = sizeof(DNSHeader) + strlen((const char*)query_name) + 1 + sizeof(DNSQuery);
  std::cout << "Wartość header->qr przed wysłaniem to " << ntohl(header->qr) << std::endl;
}

void mDNS::prepare_A_query() {

}

void mDNS::prepare_PTR_response(unsigned char *response, uint16_t ID) {
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  RRecord *record = NULL;
  header = (DNSHeader*)&response;
  header->ID = htons (rand()%65537);
  header->rd = 0;
  header->qr = 1;
  header->tc = 0;
  header->aa = 0;
  header->opcode = 0;
  header->rcode = 0;
  header->cd = 0;
  header->ad = 0;
  header->z = 0;
  header->ra = 0;
  header->qdcount = 0;
  header->ancount = htons(1);
  header->nscount = 0;
  header->arcount = 0;
  //query_name = (unsigned char*)&query_buf[sizeof(DNSHeader)];
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
      boost::asio::buffer((char*) query_buf, query_buf_length), endpoint_,
      boost::bind(&mDNS::handle_send_to, this,
        boost::asio::placeholders::error));
}

void mDNS::receive() {
  unsigned char answer[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
  socket_.async_receive_from(
      boost::asio::buffer(answer), sender_endpoint_,
      boost::bind(&mDNS::handle_receive_from, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred,
        answer));
}

void mDNS::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd, unsigned char answer[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)])
{


  if (!error && bytes_recvd > 0)
  {
    sleep(1);
    DNSHeader *header = NULL;
    DNSQuery *query = NULL;
    header = (DNSHeader*)answer;
    //header->qr = 1; //becoming response
    std::cout<<"Otrzymane ID to " << ntohs(header->ID) << std::endl;
    std::cout<<"Pakiet truncated to " << ntohs(header->tc) << std::endl;
    query_name = (unsigned char*)&answer[sizeof(DNSHeader)];

    //std::cout << "Zabieram " << strlen((const char*) query_name) << std::endl;
    query = (DNSQuery*)&query_buf[sizeof(DNSHeader) + strlen((const char*) query_name) + 1];
    //ChangetoDnsNameFormat(query_name, query_name);
    //std::cout<< "query->type = " << query->type << " + " << ntohs(query->type) <<std::endl;
    //std::cout<< "query->qclass = " << query->qclass << " + " << ntohs(query->qclass) << std::endl;
    //std::cout<< "Długość zapytania to " << strlen((char*)query_name) << std::endl;
    char translation[strlen((char*) query_name)];
    for (unsigned int i = 1; i < strlen((char*) query_name); i++) {
      int x = (int) query_name[i];
      //std::cout<< x << " ";
      if (x >= 32 && x <= 126) // check if printable
        translation[i-1] = static_cast<char>(x);//(char) std::to_string(x).c_str();
      else
        translation[i-1] = '.';
    }

    //std::cout<< "Wartość header->qr to " << ntohs(header->qr) << std::endl

    translation[strlen((char*) query_name)-1] = '.';
    std::cout << "OTRZYMANA WIADOMOŚĆ TO " << translation << std::endl;

    if (ntohs(header->tc) == 0) { // przyjmujemy tylko nie truncated
      if (ntohs(header->qr) == 0) {// query
        switch (ntohs(query->type)) {
          case 12: //PTR
            if (strcmp(translation, SERVICE_NAME) == 0) {
              std::cout << " Otrzymano zapytanie PTR \n";
              unsigned char response[sizeof(DNSHeader) + 256 + sizeof(DNSQuery)];
              prepare_PTR_response(response, ntohs(header->ID));
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
      else if (ntohs(header->qr) == 1) { // response
        switch (ntohs(query->type)) {
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
  receive();
}
