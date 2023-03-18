#ifndef DNS_GENERATOR_H
#define DNS_GENERATOR_H

#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <arpa/inet.h>
#include "DNS.h"

class DNSPacketBuilder{
	public:	
		std::vector<uint8_t> build_packet(const DNSPacket& packet);

		std::vector<uint8_t> build_response(const DNSResponse response);
};

#endif
