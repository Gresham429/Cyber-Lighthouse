#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

//package an webclient which can set the protocol(tcp or udp) and the port selfly
class WebClient{
	public:
		WebClient(boost::asio::io_service& io_service, const std::string& address, const int& port, const std::string& protocol, const std::string& message):
			address_(address),
			port_(port),
			protocol_(protocol),
			message_(message)
		{

			//cope with the message in tcp, firstly connect
			if(protocol_ == "--tcp"){
				tcp::resolver resolver(io_service);
				tcp::resolver::query query(tcp::v4(), address_, std::to_string(port_));
				tcp::resolver::iterator iterator = resolver.resolve(query);

				socket_.reset(new tcp::socket(io_service));
				boost::asio::connect(*socket_, iterator);
			}

			//cope with the message in udp without connect previously
			else if(protocol_ == "--udp"){
				udp::resolver resolver(io_service);
				udp::resolver::query query(udp::v4(), address_, std::to_string(port_));
				udp::resolver::iterator iterator = resolver.resolve(query);

				socket_udp_.reset(new udp::socket(io_service));
				socket_udp_->open(udp::v4());
				socket_udp_->send_to(boost::asio::buffer(message_), *iterator);
			}

			//cannot recognize the protocol
			else {
				std::cerr << "Unkonwn protocol" << protocol << std::endl;
			}
		}
		

		//while choose the TCP
		void send(){
			if(protocol_ == "--tcp"){
				boost::asio::write(*socket_, boost::asio::buffer(message_));
			}
			else if(protocol_ == "--udp"){
				//do not send the message again
			}
		}

		//receive
                std::string receive_tcp(){
                        //get the data length
                        uint32_t len = 0;
                        boost::asio::read(*socket_, boost::asio::buffer(&len,sizeof(len)));
                        len = ntohl(len);

                        //read data
                        std::vector<char> buf(len);
                        boost::asio::read(*socket_, boost::asio::buffer(buf));

                        return std::string(buf.begin(), buf.end());
                }

                std::string receive_udp(){
                        //read data
                        std::vector<char> buf(1024);
                        boost::asio::ip::udp::endpoint sender_endpoint;
                        size_t len = socket_udp_->receive_from(boost::asio::buffer(buf), sender_endpoint);

                        return std::string(buf.begin(), buf.begin() + len);
                }

	private:
		std::string address_;
		std::string protocol_;
		std::string message_;
		int port_;

		std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
		std::unique_ptr<boost::asio::ip::udp::socket> socket_udp_;
};

int main(int argc, char* argv[]){
	try{
		boost::asio::io_service io_service;
		
		//get the command line paramemter
		std::string ptr = argv[1];
		
		std::string address;
		
		char s_port[10];
		
		int i = 0;
		
		while(ptr[i] != ':'){
			address[i++] = ptr[i];
		}
		
		int j = 0;
	       	i++;
		
		while(ptr[i]){
			s_port[j++] = ptr[i++];
		}
		s_port[j] = '\0';

		int port = atoi(s_port);
		
		std::string protocol = argv[2];
		
		//deal with different type of message
		std::string message = argv[4];

		WebClient client(io_service, address, port, protocol, message);
		client.send();
		std::string receive = client.receive_udp();

		std::cout << "receive" << receive << std::endl;
	}catch(std::exception& e){
		std::cerr << "EXCEPTION:" << e.what() << std::endl;
	}
	return 0;
}
