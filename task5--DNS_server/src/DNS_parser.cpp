#include "../include/DNS_parser.hpp"

DNSRequest DNSParser::parseDNSRequest(const char* buffer){
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

DNSResponse DNSParser::parseDNSResponse(const char* buffer){
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

void print_query(DNSRequest request){
        //header
        std::cout << "---------------this is a query------------------" << std::endl;
        std::cout << "Transaction ID: 0x" << std::hex << request.header.ID << std::endl;
        std::cout << "Flags: 0x" << std::hex << request.header.FLAGS << " Standard query" << std::endl;
        std::cout << "--Query/Response: " << request.header.flag.qr << " = Message is a " << (request.header.flag.qr ? "Response" : "Query") << std::endl;
        std::cout << "--Opcode: " << request.header.flag.opcode << std::endl;
        //std::cout << "--Authoritative Answer: " << request.header.flag.aa << (request.header.flag.aa ? "Yes" : "No") << std::endl;
        std::cout << "--Truncated: " << request.header.flag.tc << (request.header.flag.tc ? "Yes" : "No") << std::endl;
        std::cout << "--Recursion Desired: " << request.header.flag.rd << (request.header.flag.rd ? "Yes" : "No") << std::endl;
       // std::cout << "--Recursion Available: "<< request.header.flag.ra << (request.header.flag.ra ? "Yes" : "No") << std::endl;
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
                //      std::cout << "Authority: " << std::endl;
                //      std::cout << "--Name: " << element.NAME << std::endl;
                //      std::cout << "--Type: " << element.TYPE << std::endl;
                //      std::cout << "--Class: " << element.CLASS << std::endl;
                //      std::cout << "--Time to live: " << element.TTL << std::endl;
                //      std::cout << "--RDlength: " << element.RDLENGTH << std::endl;
                //      std::cout << "--RData: " << element.RDATA << std::endl;

                //} 

                //additional
                //for(const auto& element : response.addtionals){
                //      std::cout << "Addtional: " << std::endl;
                //      std::cout << "--Name: " << element.NAME << std::endl;
                //      std::cout << "--Type: " << element.TYPE << std::endl;
                //      std::cout << "--Class: " << element.CLASS << std::endl;
                //      std::cout << "--Time to live: " << element.TTL << std::endl;
                //      std::cout << "--RDlength: " << element.RDLENGTH << std::endl;
                //      std::cout << "--RData: " << element.RDATA << std::endl;
                //} 
        }
}

