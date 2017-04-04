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

	virtual void run(void *) {
		//...
	}
};

class nemo::ThreadPool {
private:
	typedef boost::shared_ptr<ThreadPoolCallback> CallbackPtr;
	typedef boost::shared_ptr<boost::thread> ThreadPtr;

	enum Message {Run,Suspend,Halt,Block,Unblock,Invoke,CreateThread};
	enum Status {RUNNING,SUSPEND,HALT};

	struct TP_Event {
		Message msg;
		CallbackPtr callback;
	};

	struct TP_ThreadData {
		std::list<TP_Event> eventList;
		boost::shared_mutex threadLock;
		ThreadPtr threadPtr;
		Status status;

		void postEvent(const TP_Event & e) {
			eventList.push_back(e);
		}
	};
	typedef std::list<TP_ThreadData> WorkThreadList;
	typedef boost::shared_ptr<TP_ThreadData> WorkThreadArg;

	TP_ThreadData daemonData;
	WorkThreadList workThreadList;

	int minThreadNum;
	int curThreadNum;
	int maxThreadNum;
private:
	void workThread(WorkThreadArg arg) {
		TP_Event ev;
		while (true) {
			bool sleepFlag = false;
			arg->threadLock.lock_shared();
			if (arg->eventList.size()) {
				ev = *(arg->eventList.begin());
				arg->eventList.pop_front();
			}
			else {
				sleepFlag = true;
			}
			arg->threadLock.unlock_shared();

			if (!sleepFlag) {
				switch (ev.msg) {
				case Message::Invoke:

					break;
				default:
					break;
				}
			}
			else {
				boost::this_thread::sleep(boost::posix_time::millisec(30));
			}
		}
	}

	void daemonThread(void) {
		TP_Event ev;
		while (true) {
			bool isEmptyList = false;
			daemonData.threadLock.lock();
			if (daemonData.eventList.size()) {
				ev = *(daemonData.eventList.begin());
				daemonData.eventList.pop_front();
			}
			else {
				isEmptyList = true;
			}
			daemonData.threadLock.unlock();

			if (!isEmptyList) {
				switch (ev.msg) {
				case Message::CreateThread: {
						WorkThreadArg arg;
						arg->threadLock.lock();
						arg->status = Status::SUSPEND;
						arg->threadPtr = ThreadPtr(new boost::thread(boost::bind(&ThreadPool::workThread, this, arg)));
						workThreadList.push_back(*arg);
						curThreadNum += 1;
					}
					break;
				default:
					break;
				}
			}
			else {

			}
		}
	}

public:
	ThreadPool() {
		curThreadNum = 0;
		minThreadNum = 4;
		maxThreadNum = 4;
		daemonData.threadPtr = ThreadPtr(new boost::thread(boost::bind(&ThreadPool::daemonThread, this)));
		daemonData.status = Status::RUNNING;
	}

	~ThreadPool() {
		daemonData.threadPtr->join();
	}
};