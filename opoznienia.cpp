#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/thread.hpp>
#include <thread>
#include <string>
#include <ctime>

#include "err.hpp"
#include "udp_server.hpp"
#include "udp_client.hpp"
#include "tcp_client.hpp"
#include "mDNS.hpp"
#include "config.hpp"
#include "connection.hpp"

using boost::asio::ip::udp;

std::vector<Connection> connections;

void set_options(int argc, char *argv[]);

void write_connection(Connection c) {
	std::cout << "IP = " << c.ip << std::endl;
	std::cout << "credits = " << c.credits << std::endl;
	std::cout << "pos = " << c.pos << std::endl;
	std::cout << "opoznienia_ = " << c._opoznienia << std::endl;
	std::cout << "ssh_ =" << c._ssh << std::endl;
	std::cout << "udp  ssh  icmp" << std::endl;
	for (int i = 0; i < 10; i++)
		std::cout << c.udp[i] << "  " << c.ssh[i] << "  " << c.icmp[i] << std::endl;
}

int main(int argc, char *argv[]) {
	set_options(argc, argv);

	Connection con;
	con.credits = 12;
	con.alive = true;
	con._opoznienia = true;
	con._ssh = false;
	con.pos = 0;
	for (int i = 0; i < 10; i++) {
		con.udp[i] = 0; con.ssh[i] = 0; con.icmp[i] = 0;
	}
	con.ip = "localhost";
	connections.push_back(con);
	Connection con2;
	con2.credits = 6;
	con2.alive = true;
	con2._opoznienia = true;
	con2._ssh = false;
	con2.pos = 0;
	for (int i = 0; i < 10; i++) {
		con2.udp[i] = 0; con2.ssh[i] = 0; con2.icmp[i] = 0;
	}
	con.ip = "192.168.1.103";
	//connections.push_back(con);

	try
	  {
			boost::asio::io_service io_service;
			//boost::asio::signal_set signals(io_service, SIGINT, SIGTERM);
			//signals.async_wait(
    	//boost::bind(&boost::asio::io_service::stop, &io_service));

	    udp_server s(io_service, UDP_PORT);
			std::thread thread1{[&io_service](){ io_service.run(); }};
			udp_client c(io_service, std::to_string(UDP_PORT), FIND_INTERVAL);
	    std::thread thread2{[&io_service](){ io_service.run(); }};

			std::cout<<"MULTICAST START!"<<std::endl;
			mDNS Multicast(io_service,
					boost::asio::ip::address::from_string("0.0.0.0"),
					boost::asio::ip::address::from_string("224.0.0.251"),
					SERVICES_INTERVAL,
					DNS_SD
			);
			std::thread thread4{[&io_service](){ io_service.run(); }};
			//tcp_client c2(io_service, "localhost");
	    //std::thread thread3{[&io_service](){ io_service.run(); }};
						//udp_client c(io_service, "192.168.1.103", std::to_string(UDP_PORT), FIND_INTERVAL);
						//std::thread thread2{[&io_service](){ io_service.run(); }};
						//sleep(5);
						//udp_client c2(io_service, "192.168.1.103", std::to_string(UDP_PORT), FIND_INTERVAL);
						//std::thread thread3{[&io_service](){ io_service.run(); }};

			//boost::thread_group threads;

			while (true) {
				for(uint i=0; i < connections.size(); i++){
					write_connection(connections[i]);
				}
				std::cout<<"Idę spać\n";
				sleep(FIND_INTERVAL);
			}

			/*mDNS sMutlicast(io_service,
					boost::asio::ip::address::from_string("224.0.0.251"));
			std::thread thread5{[&io_service](){ io_service.run(); }};
*/
			thread1.join();
			//thread2.join();
			//thread3.join();
			thread4.join();
			//thread5.join();

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
