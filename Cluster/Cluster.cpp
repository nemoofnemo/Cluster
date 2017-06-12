#include "Cluster.h"

using namespace boost::asio;

void nemo::MasterServer::tcpLoop(void)
{
	boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 6001));
	char * receive_buffer = new char[BUF_SIZE];
	memset(receive_buffer, 0, BUF_SIZE);
	int count;
	while (!exitFlag)
	{
		boost::asio::ip::tcp::socket sock(io_service);
		acceptor.accept(sock);
		count = sock.receive(buffer(receive_buffer, BUF_SIZE));
		std::cout << "[Receive]:" << count << "bytes, from " << sock.remote_endpoint().address() << '\n';
		//sock.write_some(boost::asio::buffer(receive_buffer, BUF_SIZE));
		workLogic(sock, sock.remote_endpoint(), receive_buffer, count);
		sock.close();
		if(count > 0)
			memset(receive_buffer, 0, count);
	}
}

void nemo::MasterServer::workLogic(boost::asio::ip::tcp::socket & s, boost::asio::ip::tcp::endpoint & ep, char * data, int len)
{
	Request r;
	if (r.request.match(data, len)) {
		//register server
		if (r.request.isExist("Cmd") && r.request["Cmd"] == "reg") {
			if (r.request["Name"].size() > 0 && r.request["Port"].size() > 0) {
				ServerNode sn;
				sn.ep = ip::tcp::endpoint(ep.address(), atoi(r.request["Port"].c_str()));
				sn.name = r.request["Name"];
				serverMap.insert(std::pair<uintmax_t, ServerNode>(serverIndex, sn));
				serverIndex++;
			}
		}
	}
	else {
		//send to work server
		if (serverMap.size() > 0) {
			ip::tcp::socket client(io_service);
			client.connect(serverMap[selectServer].ep);
			client.write_some(buffer(data, len));
			char * clientData = new char[BUF_SIZE];
			memset(clientData, 0, BUF_SIZE);
			client.receive(buffer(clientData, BUF_SIZE));
			s.send(buffer(clientData, BUF_SIZE));
			selectServer++;
			selectServer %= serverMap.size();
		}
	}
	std::cout << data << std::endl;
}
