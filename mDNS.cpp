#include "mDNS.hpp"

const char* SERVICE_NAME = "_opoznienia._udp.local.";

void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host) {
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

mDNS::mDNS(boost::asio::io_service& io_service,
    const boost::asio::ip::address& multicast_address)
  : endpoint_(multicast_address, 5353),
    socket_(io_service, endpoint_.protocol()),
    timer_(io_service),
    message_count_(0)
{
  send_query();
}

void mDNS::send_query() {
  unsigned char buf[65536];
  DNSHeader *header = NULL;
  DNSQuery *query = NULL;
  header = (DNSHeader*)&buf;
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
  unsigned char *query_name;
  query_name = (unsigned char*)&buf[sizeof(DNSHeader)];
  unsigned char* DNSname = (unsigned char*) SERVICE_NAME;
  //query_name = DNSname;
  ChangetoDnsNameFormat(query_name, DNSname);
  query = (DNSQuery*)&buf[sizeof(DNSHeader) + strlen((const char*) query_name)+1];
  query->type = htons(12); // PTR
  query->qclass = htons(1);
  socket_.async_send_to(
      boost::asio::buffer((char*) buf, 4096), endpoint_,
      boost::bind(&mDNS::handle_send_to, this,
        boost::asio::placeholders::error));
}

void mDNS::handle_send_to(const boost::system::error_code& error)
{
  if (!error && message_count_ < max_message_count)
  {
    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(
        boost::bind(&mDNS::handle_timeout, this,
          boost::asio::placeholders::error));
  }
}

void mDNS::handle_timeout(const boost::system::error_code& error)
{
  /*if (!error)
  {
    std::ostringstream os;
    os << "Message " << message_count_++ << "\n";
    message_ = os.str();

    socket_.async_send_to(
        boost::asio::buffer(message_), endpoint_,
        boost::bind(&mDNS::handle_send_to, this,
          boost::asio::placeholders::error));
  }*/
}
