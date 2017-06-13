#include "FileSystem.h"
#include "boost/regex.hpp"
#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include "boost/bind.hpp"
#include "boost/algorithm/string.hpp"
#include <sstream>

using namespace std;
using namespace boost::asio;

boost::asio::io_service service;
const int BUF_SIZE = 0x200000;

void mainLoop(string ip, int port) {
	//register
	cout << "start register.\n";
	ip::tcp::socket s1(service);
	ip::tcp::endpoint ep1(ip::address::from_string(ip), 6001);
	s1.connect(ep1);
	stringstream ss;
	ss << port;
	string str1 = "Cmd:reg\r\nName:fileserver\r\nPort:"+ss.str() + "\r\n\r\n";
	s1.send(buffer(str1.c_str(), str1.size()));
	s1.close();
	cout << "register success.\nloop start.\n";
	getchar();

	//loop
	boost::asio::ip::tcp::acceptor acceptor(service, boost::asio::ip::tcp::endpoint(ip::tcp::v4(), port));
	char * receive_buffer = new char[BUF_SIZE];
	memset(receive_buffer, 0, BUF_SIZE);
	int count;
	while (true) {
		boost::asio::ip::tcp::socket sock(service);
		acceptor.accept(sock);
		count = sock.receive(buffer(receive_buffer, BUF_SIZE));
		std::cout << "[Receive]:" << count << "bytes, from " << sock.remote_endpoint().address() << '\n';
		std::cout << receive_buffer << endl;
		sock.send(buffer("this is file server."));
		sock.close();
		memset(receive_buffer, 0, BUF_SIZE);
	}

}

int main(int argc, char* argv[]) {
	int port = 6010;
	string ip = "127.0.0.1";
	if (argc == 3) {
		int tmp = atoi(argv[1]);
		if (tmp != 0) {
			port = tmp;
		}
		if (strlen(argv[2]) > 0) {
			ip = string(argv[2]);
		}
	}

	cout << "program start.\n" << ip << ':' << port << endl;
	mainLoop(ip, port);
	return 0;
}

