#ifndef PARSER_H
#define PARSER_H

#include "DNS.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <sstream>

class DNSParser{
	public:
		DNSParser() {};

		//parse the query
		DNSRequest parseDNSRequest(const char* buffer);

		//parse the response
		DNSResponse parseDNSResponse(const char* buffer);
	private:
		//parse DNS header
		DNSHeader parseHeader(const char* buffer){
			DNSHeader header;

			header.ID = ntohs(*reinterpret_cast<const uint16_t*>(buffer));
			header.FLAGS = ntohs(*reinterpret_cast<const uint16_t*>(buffer+2));
			header.flag.qr = header.FLAGS >> 15 & 1;
			header.flag.opcode = header.FLAGS >> 11 & 0xf;
			header.flag.aa = header.FLAGS >> 10 & 1;
			header.flag.tc = header.FLAGS >> 9 & 1;
			header.flag.rd = header.FLAGS >> 8 & 1;
			header.flag.ra = header.FLAGS >> 7 & 1;
			header.flag.rcode = header.FLAGS & 0xf;
			header.QDCOUNT = ntohs(*reinterpret_cast<const uint16_t*>(buffer+4));
			header.ANCOUNT = ntohs(*reinterpret_cast<const uint16_t*>(buffer+6));
			header.NSCOUNT = ntohs(*reinterpret_cast<const uint16_t*>(buffer+8));
			header.ARCOUNT = ntohs(*reinterpret_cast<const uint16_t*>(buffer+10));

			return header;
		}

		//parse DNS query
		DNSQuery parseQuery(const char* buffer, size_t& pos){
			DNSQuery query;

			int length = buffer[pos];
			pos++;

			while(length != 0){
				for(int i = 0; i < length; i++)
					query.QNAME += buffer[pos+i];
				pos += length;
				length = buffer[pos];
				if(length != 0) query.QNAME += ".";
				pos++;
			}

			query.QTYPE = ntohs(*reinterpret_cast<const uint16_t*>(buffer+pos));
			pos = pos + 2;

			query.QCLASS = ntohs(*reinterpret_cast<const uint16_t*>(buffer+pos));
			pos = pos + 2;

			return query;
		}

		//format the A rdata
		std::string formatARecord(const char* RDATA){
			std::stringstream ss;

			for(int i = 0; i < 4; i++){
				if(i != 0) ss << ".";
				ss << static_cast<int>(*reinterpret_cast<const uint8_t*>(RDATA + i));
			}

			return ss.str();
		}

		//get the DomainName
		std::string parseDomainName(const char* RDATA, const char* buffer){
			std::string domainName;
			size_t pos = 0;

			while(RDATA[pos] != 0){
				if((RDATA[pos] & 0xc0) == 0xc0){
					uint16_t offset = ntohs(*reinterpret_cast<const uint16_t*>(RDATA + pos)) & 0x3fff;
					pos += 2;
					domainName += parseDomainName(buffer + offset, buffer);
					break;
				}

				uint8_t len = RDATA[pos];
				pos++;

				for(int i = 0; i < len; i++){
					domainName += RDATA[pos + i];
				}

				pos += len;
				if(RDATA[pos] != 0) domainName += ".";
			}
			return domainName;
		}

		//format the NS rdata
		std::string formatNSRecord(const char* RDATA, const char* buffer){
			std::stringstream ss;

			//parse domain name
			std::string domainName = parseDomainName(RDATA, buffer);

			ss << domainName;

			return ss.str();
		}

		//format the CNAME rdata
		std::string formatCNAMERecord(const char* RDATA, const char* buffer){
			std::stringstream ss;

			//parse domain name
			std::string domainName = parseDomainName(RDATA, buffer);

			ss << domainName;

			return ss.str();
		}

		//parse name
		std::string parseName(const char* buffer, size_t& pos){
			std::string name;
			size_t len = buffer[pos++];

			while(len != 0){
				if((len & 0xc0) == 0xc0){
					//compressed pointer
					size_t offset = ((len & 0x3f) << 8) | buffer[pos++];
					name += parseName(buffer, offset);
					break;
				}else{
					//not compressed
					for(int i = 0; i < len; i++){
						name += buffer[pos + i];
					}
					pos += len;
					len = buffer[pos++];
					if(len != 0) name += ".";
				}
			}

			//      name.pop_back();
			return name;
		}

		//parse DNS resource record
		DNSRecord parseRecord(const char* buffer, size_t& pos){
			DNSRecord record;

			record.NAME = parseName(buffer, pos);

			record.TYPE = ntohs(*reinterpret_cast<const uint16_t*>(buffer+pos));
			pos += 2;

			record.CLASS = ntohs(*reinterpret_cast<const uint16_t*>(buffer+pos));
			pos += 2;

			record.TTL = ntohl(*reinterpret_cast<const uint32_t*>(buffer+pos));
			pos += 4;

			record.RDLENGTH = ntohs(*reinterpret_cast<const uint16_t*>(buffer+pos));
			pos += 2;

			switch(record.TYPE){
				case 1:
					record.RDATA = formatARecord(buffer + pos);
					break;
				case 2:
					record.RDATA = formatNSRecord(buffer + pos, buffer);
					break;
				case 5:
					record.RDATA = formatCNAMERecord(buffer + pos, buffer);
					break;
				default:
					record.RDATA = "We do not support this type!!! QAQ";
			}

			pos += record.RDLENGTH;

			return record;
		}
};

void print_query(DNSRequest request);
void print_response(DNSResponse response);

#endif
