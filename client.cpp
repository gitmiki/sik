//
// blocking_udp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ctime>
#include <boost/asio.hpp>

using boost::asio::ip::udp;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: blocking_udp_echo_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_service io_service;

    udp::socket s(io_service, udp::endpoint(udp::v4(), 0));

    udp::resolver resolver(io_service);
    udp::resolver::query query(udp::v4(), argv[1], argv[2]);
    udp::resolver::iterator iterator = resolver.resolve(query);

    struct timeval tv;
	  gettimeofday(&tv,NULL);
	  //return tv.tv_usec;
    boost::posix_time::ptime t1 = boost::posix_time::microsec_clock::local_time();

	  boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
    boost::uint64_t buff = htobe64(b);
    size_t request_length = sizeof(buff);

    //struct timespec start, end, diff;
    //clock_gettime(0, &start);
    s.send_to(boost::asio::buffer(&buff, request_length), *iterator);

    boost::uint64_t reply[2];
    udp::endpoint sender_endpoint;
    size_t reply_length = s.receive_from(
        boost::asio::buffer(reply, max_length), sender_endpoint);

    //clock_gettime(0, &end);
    boost::posix_time::ptime t2 = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration msdiff = t2 - t1;
    std::cout << "Wysłanie pakietu UDP i odebranie odpowiedzi zajmuje " << msdiff.total_microseconds() << " mikrosekund" << std::endl;
    std::cerr << "Sent at: " << be64toh(reply[0]) << "\n" << "Answered at: " << be64toh(reply[1]) << "\n";

	  //diff.tv_sec = end.tv_sec - start.tv_sec;
	  //diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    //std::cout << "Wysłanie pakietu UDP i odebranie odpowiedzi zajmuje " << diff.tv_sec << " sekund oraz " << diff.tv_nsec << " nanosekund\n";

    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
