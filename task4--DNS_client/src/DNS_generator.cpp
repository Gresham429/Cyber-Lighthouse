#include "../include/DNS_generator.hpp"

std::vector<uint8_t> DNSPacketBuilder::build_packet(const DNSPacket& packet){
	std::vector<uint8_t> packetbin;

	//pack the header
	packetbin.resize(12);
	uint16_t id = htons(packet.header.ID);
	uint16_t flags = htons(packet.header.FLAGS);
	uint16_t qdcount = htons(packet.header.QDCOUNT);
	uint16_t ancount = htons(packet.header.ANCOUNT);
	uint16_t nscount = htons(packet.header.NSCOUNT);
	uint16_t arcount = htons(packet.header.ARCOUNT);
	memcpy(packetbin.data(), &id, sizeof(uint16_t));
	memcpy(packetbin.data() + 2, &flags, sizeof(uint16_t));
	memcpy(packetbin.data() + 4, &qdcount, sizeof(uint16_t));
	memcpy(packetbin.data() + 6, &ancount, sizeof(uint16_t));
	memcpy(packetbin.data() + 8, &nscount, sizeof(uint16_t));
	memcpy(packetbin.data() + 10, &arcount, sizeof(uint16_t));

	//pack the query
	std::string qname = packet.query.QNAME;
	size_t qname_len = qname.length();
	size_t pos = 12;
	while(pos < qname_len + 12){
		size_t dot_pos = qname.find(".", pos-12);
		if(dot_pos == std::string::npos){
			dot_pos = qname_len;
		}
		size_t label_len = dot_pos - pos + 12;
		packetbin.push_back(static_cast<uint8_t>(label_len));
		for(size_t i = 0; i < label_len; i++){
			packetbin.push_back(static_cast<uint8_t>(qname[pos + i - 12]));
		}
		pos += label_len + 1;
	}
	packetbin.push_back(0);

	pos++;
	packetbin.resize(packetbin.size() + 4);
	uint16_t qtype = htons(packet.query.QTYPE);
	uint16_t qclass = htons(packet.query.QCLASS);
	memcpy(packetbin.data() + pos, &qtype, sizeof(uint16_t));
	memcpy(packetbin.data() + pos + 2, &qclass, sizeof(uint16_t));

	return packetbin;	
}
