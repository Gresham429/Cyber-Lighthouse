#include "../include/DNS_generator.hpp"
#include <boost/asio.hpp>
#include <sstream>

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

std::vector<uint8_t> DNSPacketBuilder::build_response(DNSResponse response){
	std::vector<uint8_t> packetbin;

	//pack the header
	packetbin.resize(12);
	uint16_t id = htons(response.header.ID);
	uint16_t flags = htons(response.header.FLAGS);
	uint16_t qdcount = htons(response.header.QDCOUNT);
	uint16_t ancount = htons(response.header.ANCOUNT);
	uint16_t nscount = htons(response.header.NSCOUNT);
	uint16_t arcount = htons(response.header.ARCOUNT);
	memcpy(packetbin.data(), &id, sizeof(uint16_t));
	memcpy(packetbin.data() + 2, &flags, sizeof(uint16_t));
	memcpy(packetbin.data() + 4, &qdcount, sizeof(uint16_t));
	memcpy(packetbin.data() + 6, &ancount, sizeof(uint16_t));
	memcpy(packetbin.data() + 8, &nscount, sizeof(uint16_t));
	memcpy(packetbin.data() + 10, &arcount, sizeof(uint16_t));

	//pack the queries
	for(int i = 0; i < response.header.QDCOUNT; i++){
		std::string qname = response.queries[i].QNAME;
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
		uint16_t qtype = htons(response.queries[i].QTYPE);
		uint16_t qclass = htons(response.queries[i].QCLASS);
		memcpy(packetbin.data() + pos, &qtype, sizeof(uint16_t));
		memcpy(packetbin.data() + pos + 2, &qclass, sizeof(uint16_t));
	}

	//pack the answers
	for(const auto& answer : response.answers){
		std::string name = answer.NAME;
		size_t name_len = name.length();
		size_t pos = packetbin.size();
		size_t tmp = pos;

		while(pos < name_len + tmp){
			size_t dot_pos = name.find(".", pos-tmp);
			if(dot_pos == std::string::npos){
				dot_pos = name_len;
			}
			size_t label_len = dot_pos - pos + tmp;
			packetbin.push_back(static_cast<uint8_t>(label_len));
			for(size_t i = 0; i < label_len; i++){
				packetbin.push_back(static_cast<uint8_t>(name[pos + i - tmp]));
			}
			pos += label_len + 1;
		}


		packetbin.push_back(0);

		pos++;
		packetbin.resize(packetbin.size() + 10);
		uint16_t type = htons(answer.TYPE);
		uint16_t class_ = htons(answer.CLASS);
		uint32_t ttl = answer.TTL;
		uint16_t rdlength = htons(answer.RDLENGTH);
		memcpy(packetbin.data() + pos, &type, sizeof(uint16_t));
		memcpy(packetbin.data() + pos + 2, &class_, sizeof(uint16_t));
		memcpy(packetbin.data() + pos + 4, &ttl, sizeof(uint32_t));
		memcpy(packetbin.data() + pos + 8, &rdlength, sizeof(uint16_t));
		pos += 10;

		if(answer.TYPE == 1){
			packetbin.resize(packetbin.size() + 4);
			std::cout << answer.RDATA << std::endl;
			std::vector<uint8_t> bytes(4);
			std::stringstream ss(answer.RDATA);
			std::string byte_str;

			while(std::getline(ss, byte_str, '.')){
				int value = std::stoi(byte_str);
				//std::cout << value << std::endl;
				uint8_t byte_value = static_cast<uint8_t>(std::min(std::max(value, 0), 255));
				//std::cout << byte_value << std::endl;
				bytes.push_back(byte_value);
			}

			for(const auto& byte : bytes){
				//std::cout << byte << " ";
				packetbin.push_back(byte);
			}
		}else if(answer.TYPE == 5){
			std::string cname = answer.RDATA;
			size_t cname_len = cname.length();
			size_t pos = packetbin.size();
			size_t tmp = pos;

			while(pos < name_len + tmp){
				size_t dot_pos = name.find(".", pos-tmp);
				if(dot_pos == std::string::npos){
					dot_pos = name_len;
				}
				size_t label_len = dot_pos - pos + tmp;
				packetbin.push_back(static_cast<uint8_t>(label_len));
				for(size_t i = 0; i < label_len; i++){
					packetbin.push_back(static_cast<uint8_t>(name[pos + i - tmp]));
				}
				pos += label_len + 1;
			}

			packetbin.push_back(0);
		}else if(answer.TYPE == 2){
			std::string name = answer.RDATA;
			size_t name_len = name.length();
			size_t pos = packetbin.size();
			size_t tmp = pos;

			while(pos < name_len + tmp){
				size_t dot_pos = name.find(".", pos-tmp);
				if(dot_pos == std::string::npos){
					dot_pos = name_len;
				}
				size_t label_len = dot_pos - pos + tmp;
				packetbin.push_back(static_cast<uint8_t>(label_len));
				for(size_t i = 0; i < label_len; i++){
					packetbin.push_back(static_cast<uint8_t>(name[pos + i - tmp]));
				}
				pos += label_len + 1;
			}


			packetbin.push_back(0);
		}
	}

	return packetbin;
}
