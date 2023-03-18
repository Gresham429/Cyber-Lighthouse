#include "../include/DNS.h"
#include "../include/DNS_generator.hpp"
#include "../include/DNS_parser.hpp"
#include "../include/server.hpp"
#include "../include/client.hpp"
#include <map>
#include <cstring>
#include <iostream>

using boost::asio::ip::udp;
using namespace std;

class DNSCache{
	public:
		void insert(const string& domain, const string& ip, const uint32_t& ttl){
			DNSRecord record = {domain, 1, 1, ttl, 4, ip};
			cache_[domain] = record;
		}

		DNSRecord lookup(const string& domain){
			auto it = cache_.find(domain);
			if(it != cache_.end()){
				return it->second;
			}

			return DNSRecord{domain, 1, 1, 0, 0, ""};
		}

	private:
		map<string, DNSRecord> cache_;
};

int main(int argc, char* argv[]){
	int port = 2000;

	WebServer server(port);
	DNSCache cache;
	DNSPacketBuilder builder;
	DNSParser parser;

	while(true){
		string message = server.receive();
		DNSRequest request = parser.parseDNSRequest(message.c_str());

		for(const auto& query : request.queries){

			DNSRecord record = cache.lookup(query.QNAME);

			if(record.RDATA.empty()){
				//can not find in cache
				boost::asio::io_service io_service;

				WebClient client(io_service, "8.8.8.8", 53, "--udp", message);
				client.send();
				string receive = client.receive_udp();

				//return
				server.send(receive);

				//write in cache
				DNSResponse response = parser.parseDNSResponse(receive.c_str());
				for(int i = 0; i < response.header.ANCOUNT; i++){
					if(response.answers[i].TYPE == 1){
						cache.insert(response.answers[i].NAME, response.answers[i].RDATA, response.answers[i].TTL);
						break;
					}
				}
			}else{
				request.header.ANCOUNT = 0x0001;
				vector<DNSQuery> QUERY(1, query);
			       	vector<DNSRecord> RECORD(1, record);	
				DNSResponse response = {request.header, QUERY, RECORD};
				vector<uint8_t> responsebin = builder.build_response(response);
				std::string sender(responsebin.begin(), responsebin.end());
				server.send(sender);
			}
		}
	}
	return 0;
}
