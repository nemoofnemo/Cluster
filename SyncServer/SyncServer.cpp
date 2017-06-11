#include <iostream>
#include <string>

#define _WIN32_WINNT  0x0501
#include <boost\asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

io_service service;
size_t read_complete(char * buff, boost::system::error_code err, size_t bytes) {
	if (err) return 0;
	bool found = std::find(buff, buff + bytes, '\n') < buff + bytes;
	// 我们一个一个读取直到读到回车，不缓存
	return found ? 0 : 1;
}

void handle_connections() {
	ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 6001));
	char buff[1024];
	while (true) {
		ip::tcp::socket sock(service);
		acceptor.accept(sock);
		int bytes = read(sock, buffer(buff), boost::bind(read_complete, buff, _1, _2));
		std::string msg(buff, bytes);
		cout << msg << endl;
		sock.write_some(buffer(msg));
		sock.close();
	}
}

int main(int argc, char* argv[]) {
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket udp_socket(io_service);
	boost::asio::ip::udp::endpoint local_add(boost::asio::ip::address::from_string("127.0.0.1"), 6004);
	
	udp_socket.open(local_add.protocol());
	udp_socket.bind(local_add);

	char receive_buffer[1024] = { 0 };
	while (true)
	{
		boost::asio::ip::udp::endpoint send_point;
		udp_socket.receive_from(boost::asio::buffer(receive_buffer, 1024), send_point);
		cout << "recv:" << receive_buffer << endl;
		udp_socket.send_to(boost::asio::buffer(receive_buffer), send_point);

		memset(receive_buffer, 0, 1024);
	}

	return 1;
}