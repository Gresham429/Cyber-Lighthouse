#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <arpa/inet.h>

using boost::asio::ip::tcp;

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

struct DNSHeader{
	uint16_t ID;//16bit identification number
	uint16_t FLAGS;//16bit flags
	flag_ flag;
	uint16_t QDCOUNT;//16bit number of questions
	uint16_t ANCOUNT;//16bit number of answer resource records
	uint16_t NSCOUNT;//15bit number of authority resource records
	uint16_t ARCOUNT;//16bit number of addtional resource records
};

//DNS query structure
struct DNSQuery{
	std::string QNAME;//query domain name
	uint16_t QTYPE;//query type
	uint16_t QCLASS;//query class
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

class DNSParser{
	public:
		DNSParser(){};

		//parse the query
		DNSRequest parseDNSRequest(const char* buffer){
			DNSRequest request;
			std::size_t pos = 0;

			//parse DNS header
			request.header = parseHeader(buffer);
			pos = pos + 12;

			//parse DNS query
			for(int i = 0; i < request.header.QDCOUNT; i++){
				DNSQuery query = parseQuery(buffer, pos);
				request.queries.push_back(query);
			}

			return request;
		}

		//parse the response
		DNSResponse parseDNSResponse(const char* buffer){
			DNSResponse response;
			std::size_t pos = 0;

			//parse DNS header
			response.header = parseHeader(buffer);
			pos = pos + 12;

			//parse DNS query
			for(int i = 0; i < response.header.QDCOUNT; i++){
				DNSQuery  query = parseQuery(buffer, pos);
				response.queries.push_back(query);
			}

			//parse DNS resource records
			for(int i = 0; i < response.header.ANCOUNT; i++){
				DNSRecord record = parseRecord(buffer,pos);
				response.answers.push_back(record);
			}

			for(int i = 0; i < response.header.NSCOUNT; i++){
				DNSRecord record = parseRecord(buffer, pos);
				response.authorities.push_back(record);
			}

			for(int i = 0; i < response.header.ARCOUNT; i++){
				DNSRecord record = parseRecord(buffer, pos);
				response.addtionals.push_back(record);
			}

			return response;
		}
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

			//if(!domainName.empty() && (domainName[domainName.length() - 1] == "."))
		//		domainName.pop_back();

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

		//	name.pop_back();
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

//-----------------------------------------print function-------------------------------------------------------
void print_query(DNSRequest request);
void print_response(DNSResponse response);
//------------------------------------------------------------------------------------------------------------

	int main(int argc, char* argv[]){
		DNSResponse response;
		DNSRequest request;

		std::ifstream file1("test", std::ios::binary);
		std::ifstream file2("response", std::ios::binary);

		if(!file1){
			std::cerr << "Failed to open the file1" << std::endl;
			return 1;
		}

		if(!file2){
			std::cerr << "Failed to open the filre" << std::endl;
			return 1;
		}

		char buffer1[1024];
		file1.read(buffer1, sizeof(buffer1));

		char buffer2[1024];
		file2.read(buffer2, sizeof(buffer2));

		DNSParser parser;
		request = parser.parseDNSRequest(buffer1);
		response = parser.parseDNSResponse(buffer2);

		//print
		print_query(request);
		print_response(response);

		//colse the file
		file1.close();
		file2.close();
	
		return 0;
	}

//-----------------------------------------print function-------------------------------------------------------
void print_query(DNSRequest request){
	//header
	std::cout << "---------------this is a query------------------" << std::endl;
	std::cout << "Transaction ID: 0x" << std::hex << request.header.ID << std::endl;
	std::cout << "Flags: 0x" << std::hex << request.header.FLAGS << " Standard query" << std::endl;
	std::cout << "--Query/Response: " << request.header.flag.qr << " = Message is a " << (request.header.flag.qr ? "Response" : "Query") << std::endl;
	std::cout << "--Opcode: " << request.header.flag.opcode << std::endl;
	std::cout << "--Authoritative Answer: " << request.header.flag.aa << (request.header.flag.aa ? "Yes" : "No") << std::endl;
	std::cout << "--Truncated: " << request.header.flag.tc << (request.header.flag.tc ? "Yes" : "No") << std::endl;
	std::cout << "--Recursion Desired: " << request.header.flag.rd << (request.header.flag.rd ? "Yes" : "No") << std::endl;
	std::cout << "--Recursion Available: "<< request.header.flag.ra << (request.header.flag.ra ? "Yes" : "No") << std::endl;
	std::cout << "Response Code: " << request.header.flag.rcode << std::endl;
	std::cout << "Questions: " << request.header.QDCOUNT << std::endl;
	std::cout << "Answer RRs: " << request.header.ANCOUNT << std::endl;
	std::cout << "Authority RRs: " << request.header.NSCOUNT << std::endl;
	std::cout << "Addtional RRs: " << request.header.ARCOUNT << std::endl;

	//queries
	for(const auto& element : request.queries){
		std::cout << "Query: " << std::endl;
		std::cout << "--Name: " << element.QNAME << std::endl;
		std::cout << "--Type: " << element.QTYPE << std::endl;
		std::cout << "--Class: " << element.QCLASS << std::endl;
	}
}

void print_response(DNSResponse response){
	//header
	std::cout << "---------------this is a response------------------" << std::endl;
	std::cout << "Transaction ID: 0x" << std::hex << response.header.ID << std::endl;
	std::cout << "Flags: 0x" << std::hex << response.header.FLAGS << " Standard query" << std::endl;
	std::cout << "--Query/Response: " << response.header.flag.qr << " = Message is a " << (response.header.flag.qr ? "Response" : "Query") << std::endl;
	std::cout << "--Opcode: " << response.header.flag.opcode << std::endl;
	std::cout << "--Authoritative Answer: " << response.header.flag.aa << (response.header.flag.aa ? "Yes" : "No") << std::endl;
	std::cout << "--Truncated: " << response.header.flag.tc << (response.header.flag.tc ? "Yes" : "No") << std::endl;
	std::cout << "--Recursion Desired: " << response.header.flag.rd << (response.header.flag.rd ? "Yes" : "No") << std::endl;
	std::cout << "--Recursion Available: "<< response.header.flag.ra << (response.header.flag.ra ? "Yes" : "No") << std::endl;
	std::cout << "Response Code: " << response.header.flag.rcode << std::endl;
	std::cout << "Questions: " << response.header.QDCOUNT << std::endl;
	std::cout << "Answer RRs: " << response.header.ANCOUNT << std::endl;
	std::cout << "Authority RRs: " << response.header.NSCOUNT << std::endl;
	std::cout << "Addtional RRs: " << response.header.ARCOUNT << std::endl;

	//query
	for(const auto& element : response.queries){
		std::cout << "Query: " << std::endl;
		std::cout << "--Name: " << element.QNAME << std::endl;
		std::cout << "--Type: " << element.QTYPE << std::endl;
		std::cout << "--Class: " << element.QCLASS << std::endl;
	}

	//answer
	for(const auto& element : response.answers){
		std::cout << "Answer: " << std::endl;
		std::cout << "--Name: " << element.NAME << std::endl;
		std::cout << "--Type: " << element.TYPE << std::endl;
		std::cout << "--Class: " << element.CLASS << std::endl;
		std::cout << "--Time to live: " << element.TTL << std::endl;
		std::cout << "--RDlength: " << element.RDLENGTH << std::endl;
		std::cout << "--RData: " << element.RDATA << std::endl;

		//authority
		//for(const auto& element : response.authorities){
		//	std::cout << "Authority: " << std::endl;
		//	std::cout << "--Name: " << element.NAME << std::endl;
		//	std::cout << "--Type: " << element.TYPE << std::endl;
		//	std::cout << "--Class: " << element.CLASS << std::endl;
		//	std::cout << "--Time to live: " << element.TTL << std::endl;
		//	std::cout << "--RDlength: " << element.RDLENGTH << std::endl;
		//	std::cout << "--RData: " << element.RDATA << std::endl;

		//} 

		//additional
		//for(const auto& element : response.addtionals){
		//	std::cout << "Addtional: " << std::endl;
		//	std::cout << "--Name: " << element.NAME << std::endl;
		//	std::cout << "--Type: " << element.TYPE << std::endl;
		//	std::cout << "--Class: " << element.CLASS << std::endl;
		//	std::cout << "--Time to live: " << element.TTL << std::endl;
		//	std::cout << "--RDlength: " << element.RDLENGTH << std::endl;
		//	std::cout << "--RData: " << element.RDATA << std::endl;
		//} 
	}
}
//------------------------------------------------------------------------------------------------------------
