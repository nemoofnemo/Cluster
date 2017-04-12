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
#include <utility>

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
		boost::shared_ptr<FileSystemCallback> callback;
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
	uintmax_t handle_index;
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

	void executeIO(FS_AsyncNode & node) {
		switch (node.status) {
		case AsyncStatus::READ:

			break;
		case AsyncStatus::WRITE:

			break;
		default:
			break;
		}
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

				//if selected handle is processing, find next one.
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

				//if handle is not in handle map, release resource.
				if (fsHandleMap.find(it->handle) == fsHandleMap.end()) {
					asyncQueue.erase(it);
					continue;
				}

				//if status == ABORT
				if (it->status == AsyncStatus::ABORT) {
					std::deque<FS_AsyncNode>::iterator temp = asyncQueue.begin();
					while (temp != end) {
						if (temp->handle == it->handle) {
							temp = asyncQueue.erase(temp);
						}
						++temp;
					}
					continue;
				}

				//get task
				node = *it;
				processingVector[index].handle = node.handle;
				processingVector[index].handle_st = node.handle_st;
				processingVector[index].status = node.status;
				asyncQueue.erase(it);
			}
			executeIO(node);
		}
	}

public:

	void init(void) {
		handle_index = 0;
	}

	FS_Handle createFileSystemHandle(const boost::filesystem::path & p, const boost::shared_ptr<FileSystemCallback> & cb) {
		boost::lock_guard<boost::shared_mutex> lg(lock);
		FS_Handle ret = handle_index;
		handle_index++;
		FS_Handle_ST st;
		st.handle = ret;
		st.fullPath = p;
		st.callback = cb;
		fsHandleMap.insert(std::pair<FS_Handle,FS_Handle_ST>(ret, st));
		return ret;
	}

	void releaseFileSystemHandle(FS_Handle h) {
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h);
			if (it != fsHandleMap.end()) {
				fsHandleMap.erase(it);
			}
		}
		//todo : post abort?
	}

	bool asyncRead(FS_Handle h, void * ptr, uintmax_t limit, boost::shared_ptr<FileSystemCallback> cb) {
		FS_AsyncNode node;
		node.status = AsyncStatus::READ;
		node.handle = h;
		node.data = (unsigned char *)ptr;
		node.dataLimit = limit;
	}

	bool asyncWrite(FS_Handle h) {

	}

	bool abortOperation(FS_Handle) {

	}
};

