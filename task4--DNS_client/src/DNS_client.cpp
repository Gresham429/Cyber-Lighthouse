#include "../include/client.hpp"
#include "../include/DNS_generator.hpp"
#include "../include/DNS_parser.hpp"
#include "../include/DNS.h"
#include <iostream>

using boost::asio::ip::udp;

int main(int argc, char* argv[]){
	try{
		DNSPacket packet;

		//header
		packet.header.ID = 0x1234;
		if(argc == 4 || strcmp(argv[4], "+recurrence") == 0) packet.header.FLAGS = 0x0100;
		else if(argc == 5 && strcmp(argv[4], "+norecurrence") == 0) packet.header.FLAGS = 0x0000;
		else {
			std::cout << "failed to idenify RD" << std::endl;
			exit(1);
		}
		packet.header.QDCOUNT = 0x0001;
		packet.header.ANCOUNT = 0x0000;
		packet.header.NSCOUNT = 0x0000;
		packet.header.ARCOUNT = 0x0000;

		//query
		packet.query.QCLASS = 0x0001;
		packet.query.QNAME = argv[3];

		if(strcmp(argv[1], "A") == 0) packet.query.QTYPE = 0x0001;
		else if(strcmp(argv[1], "NS") == 0) packet.query.QTYPE = 0x0002;
		else if(strcmp(argv[1], "CNAME") == 0) packet.query.QTYPE = 0x0005;
		else{
			std::cout << "Failed to identify the type" << std::endl;
			return 0;
		}

		//get the address and port
		std::string ptr = argv[2];

		std::string address;

		char s_port[10];

		int i = 0;

		while(*(argv[2] + i) != ':'){
			address += *(argv[2] + i);
			i++;
		}

		//std::cout << "       " << address << std::endl;

		int j = 0;
		i++;

		while(ptr[i]){
			s_port[j++] = ptr[i++];
		}
		s_port[j] = '\0';

		int port = atoi(s_port);

		//std::cout << port << std::endl;

		DNSPacketBuilder builder;
		std::vector<uint8_t> packetbin = builder.build_packet(packet);

		//-------------udp truncated------------------
		if(packetbin.size() > 512){
			if(argc == 4 || strcmp(argv[4], "+recurrence") == 0) packet.header.FLAGS = 0x0300;
			else if(argc == 5 && strcmp(argv[4], "+norecurrence") == 0) packet.header.FLAGS = 0x0200;
			packetbin = builder.build_packet(packet);
		}
		//---------------------------------------------

		boost::asio::io_service io_service;

		std::string message(packetbin.begin(),packetbin.end());
		WebClient client(io_service, address, port, "--udp", message);
		client.send();
		std::string receive = client.receive_udp();

		DNSParser parser;
		DNSRequest request = parser.parseDNSRequest(reinterpret_cast<const char*>(packetbin.data()));
		DNSResponse response = parser.parseDNSResponse(receive.c_str());

		print_query(request);
		print_response(response);

	}catch(std::exception& e){
		std::cerr << "EXCEPTION: " << e.what() << std::endl;
	}

	return 0;
}
