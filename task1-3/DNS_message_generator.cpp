#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <arpa/inet.h>

//DNS header
struct DNSHeader{
	uint16_t ID;            //identification number
	uint16_t FLAGS;         //DNS flags
	uint16_t QDCOUNT;       //number of question entries
	uint16_t ANCOUNT;       //number of answers
	uint16_t NSCOUNT;       //number of authorities
	uint16_t ARCOUNT;       //number of addtionals
};

//DNS quuery
struct DNSQuery{
	std::string QNAME;      //domain name
	uint16_t QTYPE;         //query type
	uint16_t QCLASS;        //query class
};

//DNS packet
struct DNSPacket{
	DNSHeader header;
	DNSQuery query;
};

class DNSPacketBuilder{
	public:
		//constrcutor
		DNSPacketBuilder() {}
		std::vector<uint8_t> build_packet(const DNSPacket& packet){
			std::vector<uint8_t> packetbin;

			//pack the header
			packetbin.resize(sizeof(DNSHeader));
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
			size_t pos = sizeof(DNSHeader);
			while(pos < qname_len + sizeof(DNSHeader)){
				size_t dot_pos = qname.find(".", pos-sizeof(DNSHeader));
				if(dot_pos == std::string::npos){
					dot_pos = qname_len;
				}
				size_t label_len = dot_pos - pos + sizeof(DNSHeader);
				packetbin.push_back(static_cast<uint8_t>(label_len));
				for(size_t i = 0; i < label_len; i++){
					packetbin.push_back(static_cast<uint8_t>(qname[pos + i - sizeof(DNSHeader)]));
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
};

int main(int argc, char*argv[]){
	DNSPacket packet = {
		{0x1234, 0x0001, 0x0001, 0x0000, 0x0000, 0x0000},
		{"www.baidu.com", 0x0001, 0x0001}
	};

	DNSPacketBuilder builder;
	std::vector<uint8_t> packetbin = builder.build_packet(packet);

	std::ofstream outfile("test", std::ios::out | std::ios::binary);

	if(!outfile){
		std::cout << "Failed to create file!" << std::endl;
		return 1;
	}

	outfile.write(reinterpret_cast<const char*>(&packetbin[0]), packetbin.size());

	outfile.close();

	return 0;
}
