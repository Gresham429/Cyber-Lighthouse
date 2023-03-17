#include "../include/DNS.h"
#include "../include/DNS_parser.hpp"
#include "../include/server.hpp"

using boost::asio::ip::udp;

int main(int argc, char* argv[]){
	if(argc < 3){
		std::cerr << "Usage: <filename> --<protocol> <port>" << std::endl;
		exit(1);
	}

	int max_requests_per_second = 10;
	int port = std::atoi(argv[2]);
	std::string protocol = argv[1];

	WebServer server(protocol, port, max_requests_per_second);
	server.start();

	return 0;
}
