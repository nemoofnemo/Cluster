#include "FileSystem.h"
#include "boost/regex.hpp"
#include "boost/asio.hpp"
#include "boost/thread.hpp"
#include "boost/bind.hpp"
#include "boost/algorithm/string.hpp"
#include <sstream>

using namespace std;
using namespace boost::asio;
using namespace nemo;

boost::asio::io_service service;
const int BUF_SIZE = 0x200000;

class Callback : public FileSystemIO::FileSystemIOCallback {
public:
	boost::asio::ip::tcp::socket * sock;
	string path;

	Callback() {

	}

	~Callback() {

	}

	void run(const FileSystemIO::FS_AsyncHandle_ST & ast, FileSystemIO::ErrorCode e, void * data, uintmax_t count) {
		if (count > 0) {
			int sc = sock->send(buffer((char*)data, count));
			cout << "[read]:" << sc << "bytes from " << path << endl;
		}
		else {
			sock->send(buffer("read error"));
		}
		
		if (e != FileSystemIO::ErrorCode::PENDING) {
			delete[](char *)data;
			sock->close();
			delete sock;
		}
	}
};

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

	//loop
	FileSystemIO fs;
	fs.init();
	fs.run();
	boost::asio::ip::tcp::acceptor acceptor(service, boost::asio::ip::tcp::endpoint(ip::tcp::v4(), port));
	char * receive_buffer = new char[BUF_SIZE];
	memset(receive_buffer, 0, BUF_SIZE);
	int count;
	while (true) {
		boost::asio::ip::tcp::socket * sock = new ip::tcp::socket(service);
		acceptor.accept(*sock);
		count = sock->receive(buffer(receive_buffer, BUF_SIZE - 1));
		receive_buffer[count] = 0;
		std::cout << "[Receive]:" << count << "bytes, from " << sock->remote_endpoint().address() << '\n';
		std::cout << receive_buffer << endl;
		string path(receive_buffer);
		if (boost::filesystem::is_regular_file(boost::filesystem::path(path))) {			
			FileSystemIO::FS_Handle h = fs.createFileSystemHandle(boost::filesystem::path(path));
			FileSystemIO::FS_AsyncHandle_ST ah = fs.createAsyncHandleST(h);
			boost::shared_ptr<Callback> cb(new Callback);
			cb->sock = sock;
			cb->path = path;
			fs.asyncReadAll(ah, cb, new char[BUF_SIZE], BUF_SIZE, BUF_SIZE);
		}
		else {
			sock->send(buffer("Invalid path"));
			sock->close();
			delete sock;
		}
		if(count > 0)
			memset(receive_buffer, 0, count);
		else {
			memset(receive_buffer, 0, BUF_SIZE);
		}
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

