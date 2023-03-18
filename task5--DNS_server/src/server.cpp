#include "../include/server.hpp"
#include "../include/DNS.h"
#include "../include/DNS_parser.hpp"
#include <stdexcept>

using boost::asio::ip::udp;

std::string WebServer::receive(){
	try{
		boost::asio::io_service io_service;
		//build the socker without connet previously
		udp::socket socket(io_service, udp::endpoint(udp::v4(), m_port));
		std::cout << "UDP server started on port " << m_port << std::endl;

		char data[1024];
		udp::endpoint sender_endpoint;
		boost::system::error_code error;

		//receive the data
		size_t length = socket.receive_from(boost::asio::buffer(data, 1024), sender_endpoint, 0, error);

		//throw the error when it is not representative an overflow
		if(error && error != boost::asio::error::message_size){
			throw boost::system::system_error(error);
		}

		//record the client's ip and port
		client_ip = sender_endpoint.address().to_string();
		client_port = sender_endpoint.port();

		//generate the message
		std::string message(data, length);
		
		return message;
	}catch(const std::exception& e){
		if(dynamic_cast<const std::ios_base::failure*>(&e) == nullptr)
			std::cerr << "EXCEPTION:" << e.what() << std::endl;
		exit(1);
	}
}

void WebServer::send(const std::string& message){
	try{
		boost::asio::io_service io_service;

		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), client_ip, std::to_string(client_port));
		//std::cout << client_ip << std::endl;
		udp::endpoint receiver_endpoint = *resolver.resolve(query);

		udp::socket socket(io_service, udp::endpoint(udp::v4(), 0));
		if(!socket.is_open()){
			std::cerr <<  "FALIED TO OPEN THE SOCKET" << std::endl;
			exit(1);
		}

		socket.send_to(boost::asio::buffer(message), receiver_endpoint);
		
		socket.close();
		return;
	}catch(const std::exception& e){
		if(dynamic_cast<const std::ios_base::failure*>(&e) == nullptr)
			std::cerr << "EXCEPTION: " << e.what() << std::endl;
		return;
	}
}
