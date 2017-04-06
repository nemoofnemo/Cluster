#pragma once
#include <iostream>  
#include <fstream>
#include <fstream>  
#include <ctime>
#include <string>
#include <boost/filesystem.hpp>  
#include <boost/atomic.hpp>
#include <map>
#include <list>
#include <deque>
#include <queue>

namespace nemo {
	struct FileSystemNode;
	struct FileSystemBuffer;
	class FileSystem;
	class FileSystemIO;
}

struct nemo::FileSystemNode {
	boost::filesystem::path name;
	uintmax_t size;
	std::time_t lastWriteTime;
	bool isDirectory;
};

struct nemo::FileSystemBuffer {
	void * data;
	uintmax_t size;
};

class nemo::FileSystem {
private:
	enum Message {FS_CREATE_FILE, FS_CREATE_DIRECTORY, FS_READ, FS_WRITE, FS_IS_EXIST, FS_REMOVE, FS_RENAME, FS_GET_LIST, FS_ABORT };
	enum Default {BLOCK_SIZE = 4096, TASK_QUEUE_SIZE = 2048, OPERATION_TIMEOUT = 60};

	struct FS_Task {		
		unsigned long handle;
		Message msg;
		void * ptr = NULL;
	};

	boost::atomic_ulong index;
	std::map<uintmax_t, boost::filesystem::path> handleMap;
	std::queue<FS_Task> taskQueue;

public:
	FileSystem() {

	}

	virtual ~FileSystem() {

	}

	uintmax_t read(boost::filesystem::path p, FileSystemBuffer buf) {

	}

	uintmax_t write(boost::filesystem::path p, FileSystemBuffer buf) {

	}

	void asyncRead() {

	}

	void asyncWrite() {

	}
};
