#include "Cluster.h"

void nemo::MasterServer::udpLoop(void)
{
	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket udp_socket(io_service);
	boost::asio::ip::udp::endpoint local_add(boost::asio::ip::address::from_string("127.0.0.1"), 6004);

	udp_socket.open(local_add.protocol());
	udp_socket.bind(local_add);

	char * receive_buffer = new char[BUF_SIZE];
	int count;
	while (!exitFlag)
	{
		boost::asio::ip::udp::endpoint send_point;
		count = udp_socket.receive_from(boost::asio::buffer(receive_buffer, BUF_SIZE), send_point);
		std::cout << "[Receive]:" <<count <<"bytes\n" << receive_buffer << std::endl;
		//udp_socket.send_to(boost::asio::buffer(receive_buffer, count), send_point);
		workLogic(udp_socket, send_point, receive_buffer, count);
		memset(receive_buffer, 0, count);
	}
}

void nemo::MasterServer::workLogic(boost::asio::ip::udp::socket & s, boost::asio::ip::udp::endpoint & ep, char * data, int len)
{
	Request r;
	if (!r.request.match(data, len)) {
		if (r.request.isExist("Cmd") && r.request["Cmd"] == "reg") {

		}
	}

}
