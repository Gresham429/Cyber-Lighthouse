#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <map>
#include <boost/asio.hpp>

//define a WebServer class
class WebServer{
	public:
		//receive the arguement
		WebServer(const std::string& protocol, const int&port, int max_requests_per_second):
			m_protocol(protocol), m_port(port), m_max_requests_per_second(max_requests_per_second) {}

		//start the WebServer
		void start();
	private:
		std::string m_protocol;
		int m_port;
		int m_max_requests_per_second;
};

#endif
