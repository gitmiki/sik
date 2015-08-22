#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <thread>
#include <string>
#include <ctime>

#include "err.hpp"
#include "udp_server.cpp"
#include "udp_client.cpp"
#include "tcp_client.cpp"
#include "config.hpp"

using boost::asio::ip::udp;

void set_options(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	set_options(argc, argv);

	try
	  {
			boost::asio::io_service io_service;
	    boost::asio::io_service io_service_server;
	    boost::asio::io_service io_service_client;

	    udp_server s(io_service, UDP_PORT);
			std::thread thread1{[&io_service](){ io_service.run(); }};
			udp_client c(io_service, "localhost", std::to_string(UDP_PORT));
	    std::thread thread2{[&io_service](){ io_service.run(); }};
			tcp_client c2(io_service, "localhost");
	    //io_service_server.run();

	    std::thread thread3{[&io_service](){ io_service.run(); }};

			thread1.join();
			thread2.join();
			thread3.join();
			//for (int i = 0; i < 100; i++) { std::cout << "lalalala\n"; sleep(1); }
	  }
	  catch (std::exception& e)
	  {
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
