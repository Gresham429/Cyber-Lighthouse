#ifndef DNS_H
#define DNS_H

#include <iostream>
#include <vector>
#include <cstring>

//flag
struct flag_{
        bool qr;
        int opcode;
        bool aa;
        bool tc;
        bool rd;
        bool ra;
        int rcode;
};

//DNS header
struct DNSHeader{
	uint16_t ID;            //identification number
	uint16_t FLAGS;         //DNS flags
	flag_ flag;
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

//DNS resource record stucture
struct DNSRecord{
        std::string NAME;//Domain name
        uint16_t TYPE;//resource record type
        uint16_t CLASS;//resource record class
        uint32_t TTL;//time to live
        uint16_t RDLENGTH;//length of resource data
        std::string RDATA;//resource data
};

//DNS request structure
struct DNSRequest{
        DNSHeader header;
        std::vector<DNSQuery> queries;
};

//DNS response structure
struct DNSResponse{
        DNSHeader header;
        std::vector<DNSQuery> queries;
        std::vector<DNSRecord> answers;
        std::vector<DNSRecord> authorities;
        std::vector<DNSRecord> addtionals;
};

//DNS packet
struct DNSPacket{
	DNSHeader header;
	DNSQuery query;
};

#endif
