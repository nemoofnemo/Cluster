#pragma once
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include <list>
#include <string>

namespace nemo {
	class ThreadPoolCallback;
	class ThreadPool;
}

class nemo::ThreadPoolCallback {
private:
	void operator=(const ThreadPoolCallback & cb) {
		//..
	}

	ThreadPoolCallback(const ThreadPoolCallback & cb) {
		//..
	}

public:
	virtual void run(void) {
		//...
	}
};

class nemo::ThreadPool {
public:
	enum Status { RUNNING, SUSPEND, HALT };
	
private:
	enum DEFAULT_SLEEP {TIME = 1000};

	int minThreadNum;
	int maxThreadNum;
	int curThreadNum;
	Status status;
	
	std::list<boost::shared_ptr<boost::thread>> threadList;
	boost::shared_mutex eventLock;
	std::list<boost::shared_ptr<ThreadPoolCallback>> eventList;

	void workThread(void) {
		while (status != Status::HALT) {
			if (status == Status::SUSPEND) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(DEFAULT_SLEEP::TIME));
				continue;
			}
			
			boost::shared_ptr<ThreadPoolCallback> ptr;

			eventLock.lock();
			if (eventList.size()) {
				ptr = *(eventList.begin());
				eventList.pop_front();
			}
			eventLock.unlock();

			ptr->run();
		}
	}

public:
	ThreadPool() {
		minThreadNum = 4;
		maxThreadNum = 4;
		curThreadNum = 4;
		status = Status::SUSPEND;

		for (int i = 0; i < minThreadNum; ++i) {
			threadList.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&ThreadPool::workThread, this))));
		}
	}

	virtual ~ThreadPool() {
		if (status != Status::HALT) {
			stop();
			status = Status::HALT;
		}
	}

	void setStatus(Status st) {
		if (st == Status::HALT)
			return;
		status = st;
	}

	Status getStatus(void) {
		return status;
	}

	void run(void) {
		status = Status::RUNNING;		
	}

	void stop(void) {
		status = Status::HALT;
		std::list<boost::shared_ptr<boost::thread>>::iterator it = threadList.begin();
		std::list<boost::shared_ptr<boost::thread>>::iterator end = threadList.end();
		while (it != end) {
			(*it)->join();
			it++;
		}
		threadList.clear();
		eventList.clear();
	}

	bool addEvent(const boost::shared_ptr<ThreadPoolCallback> & ptr) {
		bool ret = true;;
		eventLock.lock();
		eventList.push_back(ptr);
		eventLock.unlock();
		return ret;
	}
};