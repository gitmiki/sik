#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <string>
#include <ctime>

#include "err.hpp"
#include "udp_server.hpp"
#include "udp_client.hpp"
#include "tcp_client.hpp"
#include "icmp_client.hpp"
#include "mDNS.hpp"
#include "config.hpp"
#include "connection.hpp"

using boost::asio::ip::udp;

std::vector<Connection> connections;

void set_options(int argc, char *argv[]);

void write_connection(Connection c) {
	int udp_av = 0, ssh_av = 0, icmp_av = 0;
	std::cout << "IP = " << c.ip << std::endl;
	std::cout << "udp  ssh  icmp" << std::endl;
	for (int i = 0; i < 10; i++)
		std::cout << c.udp[i] << "  " << c.ssh[i] << "  " << c.icmp[i] << std::endl;
	int sum_udp = 0, count_udp = 0, sum_ssh = 0, count_ssh = 0, sum_icmp = 0, count_icmp = 0;
	for (int i = 0; i < 10; i++) {
		if (c.udp[i] != 0) {
			sum_udp += c.udp[i];
			count_udp++;
		}
		if (c.ssh[i] != 0) {
			sum_ssh += c.ssh[i];
			count_ssh++;
		}
		if (c.icmp[i] != 0) {
			sum_icmp += c.icmp[i];
			count_icmp++;
		}
	}
	if (count_udp != 0)
		udp_av = sum_udp / count_udp;
	if (count_ssh != 0)
		ssh_av = sum_ssh / count_ssh;
	if (count_icmp != 0)
		icmp_av = sum_icmp / count_icmp;
	std::cout << "AVERAGE:" << std::endl << udp_av << " " << ssh_av << " " << icmp_av << "\n\n";
}

int main(int argc, char *argv[]) {
	set_options(argc, argv);

	Connection con;
	con.udp_credits = 12;
	con.tcp_credits = 12;
	con.icmp_credits = 12;
	con.alive = true;
	con._opoznienia = true;
	con._ssh = true;
	con.pos_udp = 0;
	con.pos_tcp = 0;
	for (int i = 0; i < 10; i++) {
		con.udp[i] = 0; con.ssh[i] = 0; con.icmp[i] = 0;
	}
	con.ip = "localhost";
	//connections.push_back(con);
	Connection con2;
	con2.udp_credits = 6;
	con2.alive = true;
	con2._opoznienia = true;
	con2._ssh = false;
	con2.pos_udp = 0;
	for (int i = 0; i < 10; i++) {
		con2.udp[i] = 0; con2.ssh[i] = 0; con2.icmp[i] = 0;
	}
	con.ip = "192.168.1.103";
	//connections.push_back(con);

	try
	  {
			boost::asio::io_service io_service;

	    udp_server s(io_service, UDP_PORT);
			std::thread thread1{[&io_service](){ io_service.run(); }};
			udp_client c(io_service, std::to_string(UDP_PORT), FIND_INTERVAL);
	    std::thread thread2{[&io_service](){ io_service.run(); }};
			tcp_client c2(io_service, FIND_INTERVAL);
			std::thread thread3{[&io_service](){ io_service.run(); }};
			icmp_client c3(io_service, FIND_INTERVAL);
			std::thread thread4{[&io_service](){ io_service.run(); }};

			mDNS Multicast(io_service,
					boost::asio::ip::address::from_string("0.0.0.0"),
					boost::asio::ip::address::from_string("224.0.0.251"),
					SERVICES_INTERVAL,
					DNS_SD
			);
			std::thread thread5{[&io_service](){ io_service.run(); }};

			while (true) {
				for(uint i=0; i < connections.size(); i++){
					write_connection(connections[i]);
				}
				std::cout<<"sleeping, " << connections.size() << " records in vector\n";
				sleep(REFRESH_TIME);
			}

			thread1.join();
			thread2.join();
			thread3.join();
			thread4.join();
			thread5.join();

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
