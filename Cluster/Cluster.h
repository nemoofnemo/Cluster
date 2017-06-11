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
		boost::asio::ip::udp::endpoint ep;
		uintmax_t timestamp;
	};

	std::map<uintmax_t, ServerNode> serverMap;

	//request

	struct Request {
		uintmax_t serverID;
		nemo::Protocol request;
		uintmax_t timestamp;
	};

	std::map<uintmax_t, boost::shared_ptr<Request>> requestMap;

	//timer

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

	boost::shared_ptr<boost::thread> networkThread;

};