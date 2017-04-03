#include <iostream>
#include <string>

#define _WIN32_WINNT  0x0501
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

class server {
private:
	io_service service;
	ip::tcp::acceptor * acceptor;

public:
	server() {
		acceptor = new ip::tcp::acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 6001));
	}

	~server() {
		delete acceptor;
	}

	void run(void) {
		service.run();
	}

	void write_handler(boost::shared_ptr<std::string> pstr, boost::system::error_code ec, size_t bytes_transferred) {
		if (ec)
			std::cout << "send falied" << std::endl;
		else
			std::cout << *pstr << " send success" << std::endl;
	}

	void accept_handler(boost::shared_ptr<ip::tcp::socket>psocket, boost::system::error_code ec) {
		if (ec) {
			return;
		}
		boost::shared_ptr<ip::tcp::socket> ptr(new ip::tcp::socket(service));
		acceptor->async_accept(*psocket, boost::bind(&server::accept_handler, this, ptr, _1));

		std::cout << psocket->remote_endpoint().address() << std::endl;
		boost::shared_ptr<std::string> pstr(new std::string("hello async world!"));
		psocket->async_write_some(buffer(*pstr),
			boost::bind(&server::write_handler, this, pstr, _1, _2)
		);
	}

	void start() {
		boost::shared_ptr<ip::tcp::socket> psocket(new ip::tcp::socket(service));
		acceptor->async_accept(*psocket, boost::bind(&server::accept_handler, this, psocket, _1));
	}

};

int main(void) {
	server s;
	s.start();
	s.run();
	return 0;
}