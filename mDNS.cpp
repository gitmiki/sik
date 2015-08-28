#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"

const short MULTICAST_PORT = 5353;
const char* SERVICE_NAME = "_opoznienia._udp.local.";

#include "mDNS.hpp"
#include "config.hpp"

mDNS::mDNS(boost::asio::io_service& io_service,
    const boost::asio::ip::address& listen_address,
    const boost::asio::ip::address& multicast_address)
  : socket_(io_service),
    endpoint_(multicast_address, 5353),
    timer_(io_service),
    message_count_(0)
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

  send_query();

  socket_.async_receive_from(
      boost::asio::buffer(data_, max_length), sender_endpoint_,
      boost::bind(&mDNS::handle_receive_from, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void mDNS::ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) {
    int lock = 0 , i;

    for(i = 0 ; i < strlen((char*)host) ; i++)
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
    timer_.expires_from_now(boost::posix_time::seconds(SERVICES_INTERVAL));
    timer_.async_wait(
        boost::bind(&mDNS::handle_timeout, this,
          boost::asio::placeholders::error));
  }
}

void mDNS::handle_timeout(const boost::system::error_code& error)
{
  if (!error)
  {
    socket_.async_send_to(
        boost::asio::buffer((char*) query_buf, query_buf_length), endpoint_,
        boost::bind(&mDNS::handle_send_to, this,
          boost::asio::placeholders::error));
  }
}

void mDNS::send_query() {
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  header = (DNSHeader*)&query_buf;
  header->ID = (unsigned short) htons (getpid());
  header->rd = 1;
  header->tc = 0;
  header->aa = 0;
  header->opcode = 0;
  header->qr = 0;
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
  socket_.async_send_to(
      boost::asio::buffer((char*) query_buf, query_buf_length), endpoint_,
      boost::bind(&mDNS::handle_send_to, this,
        boost::asio::placeholders::error));
}


void mDNS::handle_receive_from(const boost::system::error_code& error,
    size_t bytes_recvd)
{

  if (!error)
  {
    sleep(1);
    std::cout << "Dostałem: " << data_ << " " << bytes_recvd << std::endl;

    socket_.async_receive_from(
        boost::asio::buffer(data_, max_length), sender_endpoint_,
        boost::bind(&mDNS::handle_receive_from, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
}
