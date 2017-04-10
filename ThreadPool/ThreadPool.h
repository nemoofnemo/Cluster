#pragma once
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_recursive_mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <map>
#include <list>
#include <string>

namespace nemo {
	class ThreadPoolCallback;
	class ThreadPool;
}

class nemo::ThreadPoolCallback {
public:
	void operator=(const ThreadPoolCallback & cb) {
		//..
	}

	ThreadPoolCallback(const ThreadPoolCallback & cb) {
		//..
	}

	ThreadPoolCallback() {
		//...
	}

	virtual ~ThreadPoolCallback() {

	}

	virtual void run(void) {
		//...
	}
};

class nemo::ThreadPool {
public:
	enum Status { RUNNING, SUSPEND, HALT };
	
private:
	enum DEFAULT_SLEEP {TIME = 1};

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
				boost::this_thread::sleep(boost::posix_time::milliseconds(333));
				continue;
			}
			
			boost::shared_ptr<ThreadPoolCallback> ptr;

			eventLock.lock();
			if (eventList.size()) {
				ptr = *(eventList.begin());
				eventList.pop_front();
			}
			eventLock.unlock();

			if (!ptr) {
				boost::this_thread::sleep(boost::posix_time::milliseconds(DEFAULT_SLEEP::TIME));
			}
			else {
				ptr->run();
			}
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

	ThreadPool(int min) {
		minThreadNum = min;
		maxThreadNum = min;
		curThreadNum = min;
		status = Status::SUSPEND;

		for (int i = 0; i < minThreadNum; ++i) {
			threadList.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&ThreadPool::workThread, this))));
		}
	}

	virtual ~ThreadPool() {
		if (status != Status::HALT) {
			stop();
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

class ThreadPool {
public:
	enum Status { RUNNING, SUSPEND, HALT };

private:
	int minThreadNum;
	int maxThreadNum;
	int curThreadNum;
	Status status;
	
	class TP_Semaphora {
	private:
		unsigned int count;
		boost::condition_variable condition;
		boost::mutex lock;

	public:
		TP_Semaphora() {
			count = 0;
		}

		TP_Semaphora(unsigned int cnt) : count(cnt){

		}

		void post() {
			lock.lock();
			count++;
			lock.unlock();
			condition.notify_one();
		}

		void wait() {
			boost::unique_lock<boost::mutex> _lock(lock);
			while (count == 0) {
				condition.wait(_lock);
			}
			count--;
		}
	};

	boost::shared_mutex eventLock;
	TP_Semaphora semaphora;
	std::list<boost::shared_ptr<nemo::ThreadPoolCallback>> taskList;
	std::list<boost::shared_ptr<boost::thread>> threadList;

	boost::shared_ptr<nemo::ThreadPoolCallback> getTask(void) {
		boost::shared_ptr<nemo::ThreadPoolCallback> ret;
		semaphora.wait();
		eventLock.lock();
		if (taskList.size()) {
			ret = *taskList.begin();
			taskList.pop_front();
		}
		eventLock.unlock();
		return ret;
	}

	void workThread(void) {
		while (true) {
			boost::shared_ptr<nemo::ThreadPoolCallback> task = getTask();
			if (status == Status::HALT) {
				break;
			}

			if (task == NULL) {
				continue;
			}
			
			//assume no exception
			task->run();
		}
	}
	
public:
	ThreadPool() {

	}

	virtual ~ThreadPool() {

	}

	void run() {
		status = RUNNING;
	}

	void stop() {
		status = Status::HALT;
	}

	bool postTask(boost::shared_ptr<nemo::ThreadPoolCallback> pCb) {
		if (status != Status::RUNNING) {
			return false;
		}

		eventLock.lock();
		taskList.push_back(pCb);
		eventLock.unlock();
		semaphora.post();

		return true;
	}
};