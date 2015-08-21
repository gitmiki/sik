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

enum { max_length = 10240 };

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

    using namespace std; // For strlen.
    std::cout << "Enter message: ";
    char request[max_length];
    std::cin.getline(request, max_length);
    size_t request_length = strlen(request);

    struct timeval tv;
	  gettimeofday(&tv,NULL);
	  //return tv.tv_usec;
	  boost::uint64_t b = 1000000 * tv.tv_sec + tv.tv_usec;
    boost::uint64_t buff = htobe64(b);
    request_length = sizeof(buff);
    std::cout<< "Sending b = " << b << " buff = " << buff << std::endl;

    struct timespec start, end, diff;
    clock_gettime(0, &start);
    s.send_to(boost::asio::buffer(&buff, request_length), *iterator);

    boost::uint64_t reply[2];
    udp::endpoint sender_endpoint;
    size_t reply_length = s.receive_from(
        boost::asio::buffer(reply, max_length), sender_endpoint);

    clock_gettime(0, &end);
    std::cerr << "Send time: " << reply[0] << "\n" << "Reply time: " << reply[1] << "\n";

	  diff.tv_sec = end.tv_sec - start.tv_sec;
	  diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    std::cout << "Wysłanie pakietu UDP i odebranie odpowiedzi zajmuje " << diff.tv_sec << " sekund oraz " << diff.tv_nsec << " nanosekund\n";

    std::cout << "Reply is: " << buff;
    std::cout << "\n";
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
