#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <string>
#include <ctime>

#include "err.hpp"

using boost::asio::ip::udp;

bool DEBUG = true;

int UDP_PORT = 3382; //default 3382, change with -u
int INTERFACE_PORT = 3637; //default 3637, change with -U
int FIND_INTERVAL = 1; //default 1, change with -t
int SERVICES_INTERVAL = 10; //default 10, change with -T
int REFRESH_TIME = 1; //default 1, change with -v
bool DNS_SD = false; //default faulse, change with -s

void set_options(int argc, char *argv[]);

void server(boost::asio::io_service& io_service)
{
	int max_length = 1024;
  udp::socket sock(io_service, udp::endpoint(udp::v4(), UDP_PORT));
  for (;;)
  {
    char data[max_length];
    udp::endpoint sender_endpoint;
    size_t length = sock.receive_from(
        boost::asio::buffer(data, max_length), sender_endpoint);
    sock.send_to(boost::asio::buffer(data, length), sender_endpoint);
  }
}

int main(int argc, char *argv[]) {
	set_options(argc, argv);

	boost::asio::io_service io_service;
	try {
		server(io_service);

	}
	catch(std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

void set_options(int argc, char *argv[]) {
	char c;
	while ((c = getopt(argc, argv, "u:U:t:T:v:s")) != -1) {
		switch (c) {
				case 'u':
					UDP_PORT = atoi(optarg);
					break;
				case 'U':
					INTERFACE_PORT = atoi(optarg);
					break;
				case 't':
					FIND_INTERVAL = atoi(optarg);
					break;
				case 'T':
					SERVICES_INTERVAL = atoi(optarg);
					break;
				case 'v':
					REFRESH_TIME = atoi(optarg);
					break;
				case 's':
					DNS_SD = true;
					break;
				default:
					fprintf(stderr, "Wrong arguments\n");
					break;
					exit(1);
		}
	}
	if (DEBUG)
		std::cout << "UDP_PORT = " << UDP_PORT << "\nINTERFACE_PORT = " << INTERFACE_PORT <<
		"\nFIND_INTERVAL = " << FIND_INTERVAL << "\nSERVICES_INTERVAL = " << SERVICES_INTERVAL
		<< "\nREFRESH_TIME = " << REFRESH_TIME << "\nDNS_SD = " << DNS_SD << std::endl;
}
