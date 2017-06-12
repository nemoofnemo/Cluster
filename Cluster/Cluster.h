#pragma once
#include "ClusterInclude.h"
#include "ClusterUtil.h"
#include "Protocol.h"

namespace nemo {
	class MasterServer;

}

class nemo::MasterServer {
private:
	//servers

	struct ServerNode {
		std::string name;
		boost::asio::ip::tcp::endpoint ep;
		uintmax_t timestamp;
	};
	uintmax_t serverIndex;
	std::map<uintmax_t, ServerNode> serverMap;

	//request

	struct Request {
		uintmax_t serverID;
		nemo::Protocol request;
		uintmax_t timestamp;
	};
	uintmax_t requestIndex;
	std::map<uintmax_t, boost::shared_ptr<Request>> requestMap;

	//todo:timer

	enum TimerType {TT_NULL, TT_REQUEST, TT_SERVER};
	struct TimerNode {
		TimerType type;
		uintmax_t arg;
		void * ptr;

		TimerNode() {
			type = TT_NULL;
			arg = 0;
			ptr = NULL;
		}
	};

	//todo : timer tree

	bool exitFlag;
	boost::shared_ptr<boost::thread> workThread;

	//===========================
	const int BUF_SIZE = 0x200000; //2MB
	int selectServer;
	boost::asio::io_service io_service;
	void tcpLoop(void);
	void workLogic(boost::asio::ip::tcp::socket & s, boost::asio::ip::tcp::endpoint & ep, char * data, int len);
	
public:
	MasterServer() {
		exitFlag = false;
		serverIndex = 0;
		requestIndex = 0;
		selectServer = 0;
	}

	~MasterServer() {

	}

	void run(void) {

	}

	void debugRun(void) {
		tcpLoop();
	}

	void halt(void) {

	}
};