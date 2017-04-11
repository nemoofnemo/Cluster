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
		FS_Handle_ST * pHS = NULL;
		void * data = NULL;
		uintmax_t dataLimit = 0;
		uintmax_t dataSize = 0;
		uintmax_t dataPos = 0;
		uintmax_t fileStart = 0;
		uintmax_t filePos = 0;
	};
	std::deque<FS_AsyncNode> asyncQueue;

	boost::shared_mutex lock;

	void workThread(void) {
		int index = -1;
		while (true) {
			bool callbackFlag = false;

			lock.lock();
			if (asyncQueue.size()== 0) {
				lock.unlock();
				//sleep
				continue;
			}

			FS_AsyncNode top = *asyncQueue.begin();
			if (top.status == AsyncStatus::ABORT) {
				for (int i = 0; i < processingVector.size(); ++i) {
					if (top.handle == processingVector[i].handle) {
						processingVector[i].status = AsyncStatus::ABORT;
						break;
					}
				}
				std::deque<FS_AsyncNode>::iterator it = asyncQueue.begin();
				std::deque<FS_AsyncNode>::iterator end = asyncQueue.end();
				while (it != end) {
					if (it->handle == top.handle) {
						it = asyncQueue.erase(it);
					}
					it++;
				}
				//call back?
				callbackFlag = true;
				//erase
			}
			else if (top.status == AsyncStatus::ERROR) {
				for (int i = 0; i < processingVector.size(); ++i) {
					if (top.handle == processingVector[i].handle) {
						processingVector[i].status = AsyncStatus::ERROR;
						break;
					}
				}
				std::deque<FS_AsyncNode>::iterator it = asyncQueue.begin();
				std::deque<FS_AsyncNode>::iterator end = asyncQueue.end();
				while (it != end) {
					if (it->handle == top.handle) {
						it = asyncQueue.erase(it);
					}
					it++;
				}
				//callback
				callbackFlag = true;
				//erase
			}
			lock.unlock();

			if (callbackFlag) {


				continue;
			}

		}

	}

public:

	static FS_Handle_ST createFileSystemHandle() {

	}

	bool asyncRead(boost::filesystem::path p) {

	}
};