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
		void insert(const string& domain,const string& name,  const string& ip, const uint32_t& ttl){
			DNSRecord record = {name, 1, 1, ttl, 4, ip};
			if(cache_.find(domain) == cache_.end()) cache_[domain] = vector<DNSRecord>();
			cache_[domain].push_back(record);
		}

		vector<DNSRecord> lookup(const string& domain){
			auto it = cache_.find(domain);
			if(it != cache_.end()){
				return it->second;
			}

			return vector<DNSRecord>(1,{domain, 1, 1, 0, 0, ""});
		}

	private:
		map<string, vector<DNSRecord>> cache_;
};

int main(int argc, char* argv[]){
	int port = 53;

	WebServer server(port);
	DNSCache cache;
	DNSPacketBuilder builder;
	DNSParser parser;

	while(true){
		string message = server.receive();
		DNSRequest request = parser.parseDNSRequest(message.c_str());

		for(const auto& query : request.queries){

			vector<DNSRecord> record_ = cache.lookup(query.QNAME);

			for(const auto& record : record_){
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
						if(response.answers[i].TYPE == 0x0001){
							cache.insert(query.QNAME, response.answers[i].NAME, response.answers[i].RDATA, response.answers[i].TTL);
							break;
						}
					}
				}else{
					request.header.ANCOUNT = cache.lookup(query.QNAME).size();
					vector<DNSQuery> QUERY(1, query);
					vector<DNSRecord> RECORD = cache.lookup(query.QNAME);
					DNSResponse response = {request.header, QUERY, RECORD};
					vector<uint8_t> responsebin = builder.build_response(response);
					std::string sender(responsebin.begin(), responsebin.end());
					server.send(sender);
				}
			}
		}
	}
	return 0;
}
