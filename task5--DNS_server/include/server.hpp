#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>

//define a WebServer class
class WebServer{
	public:
		//receive the arguement
		WebServer(const int&port): m_port(port) {}

		//receive the message
		std::string receive();

		//send the message
		void send(const std::string& message);
	private:
		int m_port;
		int client_port;
		std::string client_ip;
};

#endif
