#pragma once
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <deque>

namespace nemo {
	class ThreadPoolCallback;
	class ThreadPool;
}

class nemo::ThreadPoolCallback {
public:
	ThreadPoolCallback() {

	}

	ThreadPoolCallback(const ThreadPoolCallback & cb) {
		//..
	}

	virtual ~ThreadPoolCallback() {

	}

	virtual void operator=(const ThreadPoolCallback & cb) {
		//..
	}

	virtual void operator()(void) {
		//...
	}
};

class nemo::ThreadPool {
private:
	typedef boost::shared_ptr<ThreadPoolCallback> CallbackPtr;
	typedef boost::shared_ptr<boost::thread> ThreadPtr;

	enum Message {Run,Suspend,Halt,Block,Unblock,Invoke,CreateThread};
	

	struct TP_Event {
		Message msg;
		CallbackPtr callback;
	};

	struct TP_ThreadData {
		std::list<TP_Event> eventList;
		boost::shared_mutex threadLock;
		ThreadPtr threadPtr;

		void postEvent(const TP_Event & e) {
			eventList.push_back(e);
		}
	};

	typedef std::list<TP_ThreadData> WorkThreadList;

	TP_ThreadData daemonData;
	WorkThreadList workThreadList;

	void daemonThread(void) {
		TP_Event ev;
		while (true) {
			daemonData.threadLock.lock_shared();
			ev = *(daemonData.eventList.begin());
			daemonData.eventList.pop_front();
			daemonData.threadLock.unlock_shared();

			switch (ev.msg) {
			case Message::CreateThread:

				break;
			default:
				break;
			}
		}
	}

public:
	ThreadPool() {
		daemonData.threadPtr = ThreadPtr(new boost::thread(&ThreadPool::daemonThread, this));
	}

	~ThreadPool() {
		daemonData.threadPtr->join();
	}
};