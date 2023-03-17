#include "../include/client.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

void WebClient::send(){
	if(protocol_ == "--tcp"){
		boost::asio::write(*socket_, boost::asio::buffer(message_));
	}
	else if(protocol_ == "--udp"){
		//do not send the message again
	}
}
