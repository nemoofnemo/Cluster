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

class FileSystem {
private:
	enum AsyncStatus { NONE, WRITE, READ, ABORT, ERROR };

	typedef uintmax_t FS_Handle;

	struct FS_Handle_ST {
		FS_Handle handle;
		boost::filesystem::path fullPath;
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
			}
			else if (top.status == AsyncStatus::ERROR) {
				for (int i = 0; i < processingVector.size(); ++i) {
					if (top.handle == processingVector[i].handle) {
						processingVector[i].status = AsyncStatus::ABORT;
						break;
					}
				}
			}
			lock.unlock();
		}

	}

public:
	bool asyncRead(boost::filesystem::path p) {

	}
};