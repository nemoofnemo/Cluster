#pragma once
#include <iostream>  
#include <fstream>
#include <fstream>  
#include <ctime>
#include <string>
#include <boost/filesystem.hpp>  
#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/lock_types.hpp>
#include <map>
#include <list>
#include <deque>
#include <queue>
#include <vector>

namespace nemo {
	struct FileSystemNode;
	struct FileSystemBuffer;
	class FileSystem;
	class FileSystemIO;
}

class FileSystemCallback {
public:
	void operator=(const FileSystemCallback & cb) {
		//..
	}

	FileSystemCallback(const FileSystemCallback & cb) {
		//..
	}

	FileSystemCallback() {
		//...
	}

	virtual ~FileSystemCallback() {

	}

	virtual void run(void) {
		//...
	}
};

class FileSystem {
private:
	enum QueueOperation {PUSH_BACK,PUSH_FRONT};
	enum AsyncStatus { NONE, WRITE, READ, ABORT, ERROR };

	class FS_Semaphora {
	private:
		unsigned int count;
		boost::condition_variable condition;
		boost::mutex lock;

	public:
		FS_Semaphora() {
			count = 0;
		}

		FS_Semaphora(unsigned int cnt) : count(cnt) {

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

	typedef uintmax_t FS_Handle;

	struct FS_Handle_ST {
		FS_Handle handle = 0;
		boost::filesystem::path fullPath;
		FileSystemCallback * callback = NULL;
	};
	
	std::map<FS_Handle, FS_Handle_ST> fsHandleMap;

	struct FS_Thread_Proceing {
		FS_Handle handle = 0;
		FS_Handle_ST * handle_st = NULL;
		AsyncStatus status = AsyncStatus::NONE;
	};
	std::vector<FS_Thread_Proceing> processingVector;

	struct FS_AsyncNode {
		AsyncStatus status = AsyncStatus::NONE;
		FS_Handle handle = 0;
		FS_Handle_ST * handle_st = NULL;
		unsigned char * data = NULL;
		uintmax_t dataLimit = 0;
		uintmax_t dataSize = 0;
		uintmax_t dataPos = 0;
		uintmax_t fileStart = 0;
		uintmax_t filePos = 0;
	};
	std::deque<FS_AsyncNode> asyncQueue;

	boost::shared_mutex lock;
	FS_Semaphora semaphora;
	std::list<boost::shared_ptr<boost::thread>> threadList;

private:
	
	void postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op = QueueOperation::PUSH_BACK) {
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			if (op == QueueOperation::PUSH_BACK) {
				asyncQueue.push_back(node);
			}
			else {
				asyncQueue.push_front(node);
			}
		}
		semaphora.post();
	}

	void workThread(int index) {
		while (true) {
			bool callbackFlag = false;
			
			while (asyncQueue.size() == 0) {
				semaphora.wait();
			}

			FS_AsyncNode node;
			{
				boost::lock_guard<boost::shared_mutex> lg(lock);
				if (asyncQueue.size() == 0) {
					lock.unlock();
					continue;
				}

				std::deque<FS_AsyncNode>::iterator it = asyncQueue.begin();
				std::deque<FS_AsyncNode>::iterator end = asyncQueue.end();
				while (it != end) {
					int i = 0;
					for (; i < processingVector.size(); ++i) {
						if (processingVector[i].handle == it->handle) {
							break;
						}
					}
					if (i == processingVector.size()) {
						break;
					}
					++it;
				}

				if (it == end) {
					continue;
				}

				node = *it;
				processingVector[index].handle = node.handle;
				processingVector[index].handle_st = node.handle_st;
				processingVector[index].status = node.status;
				asyncQueue.erase(it);
			}

			switch (node.status){
			default:
				break;
			}
		}

	}

public:

	static FS_Handle_ST createFileSystemHandle() {

	}

	bool asyncRead(boost::filesystem::path p) {

	}
};