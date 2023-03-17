#include "../include/server.hpp"
#include "../include/DNS.h"
#include "../include/DNS_parser.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

void WebServer::start(){
	try{
		boost::asio::io_service io_service;

		//tcp server
		if(m_protocol == "--tcp"){
			tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), m_port));
			std::cout << "TCP server started on port" << m_port << std::endl;

			std::map<std::string, int> client_request_count;//a counter to record the frequency of requests from the same ip

			while(true){
				tcp::socket socket(io_service);
				acceptor.accept(socket);

				//record the client's ip
				std::string client_ip = socket.remote_endpoint().address().to_string();
				//set the key-value
				if(client_request_count.find(client_ip) == client_request_count.end())
					client_request_count[client_ip] = 0;


				//judge the frequency
				if(client_request_count[client_ip] >= m_max_requests_per_second){
					std::string error_message = "Error: request frequency is too high";
					boost::asio::write(socket, boost::asio::buffer(error_message));
					std::cout << "Error: requested frequncy is high! [" << client_ip << "]" << std::endl;
					//reset the counter
					client_request_count[client_ip] = 0;
				}else{
					client_request_count[client_ip]++;//count++

					//read into a stream buffer
					boost::asio::streambuf request_buffer;
					boost::system::error_code error;
					boost::asio::read_until(socket, request_buffer, "\n", error);

					//ignore the error of EOF
					if(error == boost::asio::error::eof){
					}else if(error){
						throw boost::system::system_error(error);
					}

					//point to thr stream buffer above in order to analyze the message
					std::istream is(&request_buffer);
					std::string request;
					//get the request message
					std::getline(is, request);

					std::cout << "Receive message: " << request << " from [" << client_ip << "]" << std::endl;

					//request an extension for one second in order to release the pressure
					std::this_thread::sleep_for(std::chrono::seconds(1));

					//ignore the error status and transmit  the message after set the message into the stream buffer
					boost::system::error_code ignored_error;
					boost::asio::write(socket, boost::asio::buffer(request), ignored_error);
				}
			}
		}else if(m_protocol == "--udp"){
			//build the socker without connet previously
			udp::socket socket(io_service, udp::endpoint(udp::v4(), m_port));
			std::cout << "UDP server started on port " << m_port << std::endl;

			std::map<std::string, int> client_request_count;//a counter to record the frequency of requests from the same ip

			while(true){
				char data[1024];
				udp::endpoint sender_endpoint;
				boost::system::error_code error;

				//receive the data
				size_t length = socket.receive_from(boost::asio::buffer(data, 1024), sender_endpoint, 0, error);

				//throw the error when it is not representative an overflow
				if(error && error != boost::asio::error::message_size){
					throw boost::system::system_error(error);
				}

				//record the client's ip
				std::string client_ip = sender_endpoint.address().to_string();
				//set the key-value
				if(client_request_count.find(client_ip) == client_request_count.end())
					client_request_count[client_ip] = 0;

				//judge the frequency
				if(client_request_count[client_ip] >= m_max_requests_per_second){
					std::string error_message = "Error: request frequency is too high";
					socket.send_to(boost::asio::buffer(error_message), sender_endpoint);
					std::cout << "Error: requested frequncy is high! [" << client_ip << "]" << std::endl;

					//reset the counter
					client_request_count[client_ip] = 0;
				}else{
					client_request_count[client_ip]++;//count++

					//generate the message
					std::string message(data, length);
					DNSParser parser;
					DNSResponse response = parser.parseDNSResponse(const_cast<char*>(message.c_str()));
					print_response(response);
					//std::cout << "Receive message: " << message << " from [" << client_ip << "]" << std::endl;

					//request an extension for one second in order to release the pressure
					std::this_thread::sleep_for(std::chrono::seconds(1));

					//transmit the message back
					socket.send_to(boost::asio::buffer(message), sender_endpoint);
				}
			}
		}else{
			std::cerr << "Protocol not recognized!" << std::endl;
		}
	}catch(const std::exception& e){
		if(dynamic_cast<const std::ios_base::failure*>(&e) == nullptr)
			std::cerr << "EXCEPTION:" << e.what() << std::endl;
		return;
	}
}
