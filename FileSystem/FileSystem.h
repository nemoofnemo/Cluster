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

class nemo::FileSystemIO {
public:
	enum QueueOperation { PUSH_BACK, PUSH_FRONT };
	enum AsyncStatus { NONE, APPEND_WRITE, WRITE, READ, READ_ALL, ABORT, ERROR, EXIT };
	enum ErrorCode { DONE, PENDING, END_OF_FILE, OPEN_FAIL, BAD_STREAM, IO_FAIL, UNKNOWN_ERROR };

	typedef uintmax_t FS_Handle;
	typedef uintmax_t FS_AsyncHandle;

	struct FS_AsyncHandle_ST {
		FS_Handle fileHandle = 0;
		FS_AsyncHandle asyncHandle = 0;
		AsyncStatus status = AsyncStatus::NONE;
	};

	struct FS_Handle_ST {
		FS_Handle handle = 0;
		boost::filesystem::path fullPath;
	};

	class FileSystemIOCallback {
	public:
		void operator=(const FileSystemIOCallback & cb) {
			//..
		}

		FileSystemIOCallback(const FileSystemIOCallback & cb) {
			//..
		}

		FileSystemIOCallback() {
			//...
		}

		virtual ~FileSystemIOCallback() {

		}

		virtual void run(const FS_AsyncHandle_ST & ast, ErrorCode e, void * data, uintmax_t count) {
			//...
		}
	};

private:
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

	std::map<FS_Handle, FS_Handle_ST> fsHandleMap;

	struct FS_Thread_Proceing {
		FS_Handle handle = 0;
		FS_AsyncHandle asyncHandle = 0;
		FS_Handle_ST * handle_st = NULL;
		AsyncStatus status = AsyncStatus::NONE;
	};

	struct FS_AsyncNode {
		AsyncStatus status = AsyncStatus::NONE;
		FS_AsyncHandle asyncHandle = 0;
		FS_Handle handle = 0;
		FS_Handle_ST * handle_st = NULL;
		unsigned char * data = NULL;
		uintmax_t dataLimit = 0;
		uintmax_t dataSize = 0;
		uintmax_t dataPos = 0;
		uintmax_t fileStart = 0;
		uintmax_t filePos = 0;
		boost::shared_ptr<FileSystemIOCallback> callback;
	};

	std::vector<FS_Thread_Proceing> processingVector;
	std::deque<FS_AsyncNode> asyncQueue;
	boost::shared_mutex lock;
	FS_Semaphora semaphora;
	int threadNum;
	std::list<boost::shared_ptr<boost::thread>> threadList;
	uintmax_t handleIndex;
	FS_AsyncHandle asyncHandleIndex;
	uintmax_t blockSize;
	bool exitFlag;

private:

	void postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op = QueueOperation::PUSH_BACK);

	void _postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op = QueueOperation::PUSH_BACK);

	void doRead(FS_AsyncNode & node, const int & index);

	void doReadAll(FS_AsyncNode & node, const int & index);

	void doWrite(FS_AsyncNode & node, const int & index);

	void doAppendWrite(FS_AsyncNode & node, const int & index);

	void doAbort(FS_AsyncNode & node, const int & index);

	void doUnknownError(FS_AsyncNode & node, const int & index);

	void doExit(FS_AsyncNode & node, const int & index);

	void executeIO(FS_AsyncNode & node, const int & index);

	void workThread(int index);

private:
	FileSystemIO(const FileSystemIO & fs) {

	}

	void operator=(const FileSystemIO & fs) {

	}

public:

	FileSystemIO() {

	}

	~FileSystemIO() {

	}

	void init(void);

	void debugRun(void);

	void run(void);

	void stop(void);

	FS_Handle createFileSystemHandle(const boost::filesystem::path & p);

	void releaseFileSystemHandle(FS_Handle h);

	FS_AsyncHandle_ST createAsyncHandleST(FS_Handle h);

	bool asyncRead(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t limit, uintmax_t offset, uintmax_t size);

	bool asyncReadAll(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t limit, uintmax_t size);

	bool asyncAppendWrite(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t size);

	bool asyncWrite(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t limit, uintmax_t offset, uintmax_t size);

	bool abortAsyncOperation(FS_AsyncHandle_ST & h);
};

