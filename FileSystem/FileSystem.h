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

class FileSystem {
public:
	enum QueueOperation { PUSH_BACK, PUSH_FRONT };
	enum AsyncStatus { NONE, APPEND_WRITE, WRITE, READ, READ_ALL, ABORT, ERROR };
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
		boost::shared_ptr<FileSystemCallback> callback;
	};

	std::vector<FS_Thread_Proceing> processingVector;
	std::deque<FS_AsyncNode> asyncQueue;
	boost::shared_mutex lock;
	FS_Semaphora semaphora;
	std::list<boost::shared_ptr<boost::thread>> threadList;
	uintmax_t handleIndex;
	FS_AsyncHandle asyncHandleIndex;
	uintmax_t blockSize;

private:

	void postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op = QueueOperation::PUSH_BACK) {
		if (op == QueueOperation::PUSH_BACK) {
			asyncQueue.push_back(node);
		}
		else {
			asyncQueue.push_front(node);
		}
	}

	void _postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op = QueueOperation::PUSH_BACK) {
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

	void doRead(FS_AsyncNode & node, const int & index) {
		std::fstream file;
		file.open(node.handle_st->fullPath.string(), std::ios::binary | std::ios::in);
		if (!file.is_open()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::OPEN_FAIL, node.data, 0);
			return;
		}
		file.seekg(node.fileStart + node.dataPos);
		uintmax_t target = (node.dataSize > blockSize) ? blockSize : node.dataSize;
		file.read((char *)node.data, target);
		uintmax_t cnt = file.gcount();
		node.dataPos += cnt;

		if (file.good()) {
			if (node.dataPos == node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::DONE, node.data, cnt);
				file.close();
				return;
			}
			if (node.dataPos < node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::PENDING, node.data, cnt);
				file.close();
				{
					boost::lock_guard<boost::shared_mutex> lg(lock);
					asyncQueue.push_back(node);
				}
				semaphora.post();
				return;
			}
		}

		if (file.eof()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::END_OF_FILE, node.data, cnt);
			file.close();
			return;
		}
		else if (file.fail()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::IO_FAIL, node.data, cnt);
			file.close();
			return;
		}
		else if (file.bad()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::BAD_STREAM, node.data, cnt);
			file.close();
			return;
		}
		else {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, cnt);
			file.close();
			return;
		}

		return;
	}

	void doReadAll(FS_AsyncNode & node, const int & index) {
		std::fstream file;
		file.open(node.handle_st->fullPath.string(), std::ios::binary | std::ios::in);
		if (!file.is_open()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::OPEN_FAIL, node.data, 0);
			return;
		}
		file.seekg(node.fileStart + node.dataPos);
		uintmax_t target = (node.dataSize > blockSize) ? blockSize : node.dataSize;
		file.read((char *)node.data, target);
		uintmax_t cnt = file.gcount();
		node.dataPos += cnt;

		if (file.good()) {
			if (node.dataPos == node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::DONE, node.data, cnt);
				file.close();
				return;
			}
			if (node.dataPos < node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				//run callback only all data has been read.
				/*if (node.callback != NULL)
					node.callback->run(st, ErrorCode::PENDING, node.data, cnt);*/
				file.close();
				{
					boost::lock_guard<boost::shared_mutex> lg(lock);
					asyncQueue.push_back(node);
				}
				semaphora.post();
				return;
			}
		}

		if (file.eof()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::END_OF_FILE, node.data, cnt);
			file.close();
			return;
		}
		else if (file.fail()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::IO_FAIL, node.data, cnt);
			file.close();
			return;
		}
		else if (file.bad()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::BAD_STREAM, node.data, cnt);
			file.close();
			return;
		}
		else {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, cnt);
			file.close();
			return;
		}

		return;
	}

	void doWrite(FS_AsyncNode & node, const int & index) {
		std::fstream file;
		file.open(node.handle_st->fullPath.string(), std::ios::binary | std::ios::out);
		if (!file.is_open()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::OPEN_FAIL, node.data, 0);
			return;
		}
		file.seekp(node.fileStart + node.dataPos);
		uintmax_t target = (node.dataSize > blockSize) ? blockSize : node.dataSize;
		file.write((char *)node.data, target);
		uintmax_t cnt = file.gcount();
		node.dataPos += cnt;

		if (file.good()) {
			if (node.dataPos == node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::DONE, node.data, cnt);
				file.close();
				return;
			}
			if (node.dataPos < node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::PENDING, node.data, cnt);
				file.close();
				{
					boost::lock_guard<boost::shared_mutex> lg(lock);
					asyncQueue.push_back(node);
				}
				semaphora.post();
				return;
			}
		}

		if (file.eof()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::END_OF_FILE, node.data, cnt);
			file.close();
			return;
		}
		else if (file.fail()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::IO_FAIL, node.data, cnt);
			file.close();
			return;
		}
		else if (file.bad()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::BAD_STREAM, node.data, cnt);
			file.close();
			return;
		}
		else {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, cnt);
			file.close();
			return;
		}

		return;
	}

	void doAppendWrite(FS_AsyncNode & node, const int & index) {
		std::fstream file;
		file.open(node.handle_st->fullPath.string(), std::ios::binary | std::ios::out | std::ios::ate);
		if (!file.is_open()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::OPEN_FAIL, node.data, 0);
			return;
		}

		uintmax_t target = (node.dataSize > blockSize) ? blockSize : node.dataSize;
		file.write((char *)node.data, target);
		uintmax_t cnt = file.gcount();
		node.dataPos += cnt;

		if (file.good()) {
			if (node.dataPos == node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::DONE, node.data, cnt);
				file.close();
				return;
			}
			if (node.dataPos < node.dataSize) {
				FS_AsyncHandle_ST st;
				st.asyncHandle = node.asyncHandle;
				st.fileHandle = node.handle;
				st.status = node.status;
				if (node.callback != NULL)
					node.callback->run(st, ErrorCode::PENDING, node.data, cnt);
				file.close();
				{
					boost::lock_guard<boost::shared_mutex> lg(lock);
					asyncQueue.push_back(node);
				}
				semaphora.post();
				return;
			}
		}

		if (file.eof()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::END_OF_FILE, node.data, cnt);
			file.close();
			return;
		}
		else if (file.fail()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::IO_FAIL, node.data, cnt);
			file.close();
			return;
		}
		else if (file.bad()) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::BAD_STREAM, node.data, cnt);
			file.close();
			return;
		}
		else {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			if (node.callback != NULL)
				node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, 0);
			file.close();
			return;
		}

		return;
	}

	void doAbort(FS_AsyncNode & node, const int & index) {
		if (node.callback != NULL) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			node.callback->run(st, ErrorCode::DONE, node.data, 0);
		}
	}

	void doUnknownError(FS_AsyncNode & node, const int & index) {
		if (node.callback != NULL) {
			FS_AsyncHandle_ST st;
			st.asyncHandle = node.asyncHandle;
			st.fileHandle = node.handle;
			st.status = node.status;
			node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, 0);
		}
	}

	void executeIO(FS_AsyncNode & node, const int & index) {
		switch (node.status) {
		case AsyncStatus::READ:
			doRead(node, index);
			break;
		case AsyncStatus::READ_ALL:
			doReadAll(node, index);
			break;
		case AsyncStatus::WRITE:
			doWrite(node, index);
			break;
		case AsyncStatus::APPEND_WRITE:
			doAppendWrite(node, index);
			break;
		case AsyncStatus::ABORT:
			doAbort(node, index);
			break;
		default:
			doUnknownError(node, index);
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
					size_t i = 0;
					for (; i < processingVector.size(); ++i) {
						if (processingVector[i].handle == it->handle) {
							if (processingVector[i].status == NONE) {
								continue;
							}
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
						if (temp->asyncHandle == it->asyncHandle) {
							temp = asyncQueue.erase(temp);
						}
						++temp;
					}
				}

				//get task
				node = *it;
				processingVector[index].handle = node.handle;
				processingVector[index].handle_st = node.handle_st;
				processingVector[index].status = node.status;
				processingVector[index].asyncHandle = node.asyncHandle;
				asyncQueue.erase(it);
			}
			executeIO(node, index);
			processingVector[index].status = AsyncStatus::NONE;
		}
	}

public:

	void init(void) {
		handleIndex = 0;
		asyncHandleIndex = 0;
		blockSize = 4096;
	}

	void debugRun() {
		//blockSize = 1;
		FS_Thread_Proceing tp;
		processingVector.push_back(tp);
		workThread(0);
	}

	FS_Handle createFileSystemHandle(const boost::filesystem::path & p) {
		boost::lock_guard<boost::shared_mutex> lg(lock);
		FS_Handle ret = handleIndex;
		handleIndex++;
		FS_Handle_ST st;
		st.handle = ret;
		st.fullPath = p;
		fsHandleMap.insert(std::pair<FS_Handle, FS_Handle_ST>(ret, st));
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

	FS_AsyncHandle_ST createAsyncHandleST(FS_Handle h) {
		FS_AsyncHandle_ST ret;
		ret.fileHandle = h;
		return ret;
	}

	bool asyncRead(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemCallback> & cb, void * ptr, uintmax_t limit, uintmax_t offset, uintmax_t size) {
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h.fileHandle);
			if (it == fsHandleMap.end()) {
				return false;
			}

			h.asyncHandle = asyncHandleIndex;
			h.status = AsyncStatus::READ;
			asyncHandleIndex++;

			FS_AsyncNode node;
			node.status = AsyncStatus::READ;
			node.handle = h.fileHandle;
			node.asyncHandle = h.asyncHandle;
			node.data = (unsigned char *)ptr;
			node.dataLimit = limit;
			node.handle_st = &it->second;
			node.fileStart = offset;
			node.filePos = offset;
			node.dataSize = size;
			node.dataPos = 0;
			node.callback = cb;
			postToAsyncQueue(node);
		}

		semaphora.post();
		return true;
	}

	bool asyncReadAll(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemCallback> & cb, void * ptr, uintmax_t limit, uintmax_t size) {
		if (limit < size) {
			return false;
		}
		
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h.fileHandle);
			if (it == fsHandleMap.end()) {
				return false;
			}

			h.asyncHandle = asyncHandleIndex;
			asyncHandleIndex++;
			h.status = AsyncStatus::READ_ALL;

			FS_AsyncNode node;
			node.status = AsyncStatus::READ_ALL;
			node.handle = h.fileHandle;
			node.asyncHandle = h.asyncHandle;
			node.data = (unsigned char *)ptr;
			node.dataLimit = limit;
			node.handle_st = &it->second;
			node.fileStart = 0;
			node.filePos = 0;
			node.dataSize = size;
			node.dataPos = 0;
			node.callback = cb;
			postToAsyncQueue(node);
		}

		semaphora.post();
		return true;
	}

	bool asyncAppendWrite(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemCallback> & cb, void * ptr, uintmax_t size) {
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h.fileHandle);
			if (it == fsHandleMap.end()) {
				return false;
			}
			h.asyncHandle = asyncHandleIndex;
			asyncHandleIndex++;
			h.status = AsyncStatus::APPEND_WRITE;

			FS_AsyncNode node;
			node.status = AsyncStatus::APPEND_WRITE;
			node.handle = h.fileHandle;
			node.asyncHandle = h.asyncHandle;
			node.handle_st = &it->second;
			node.callback = cb;
			node.data = (unsigned char *)ptr;
			node.dataLimit = size;
			node.dataSize = size;
			node.dataPos = 0;
			node.fileStart = 0;
			node.filePos = 0;
			postToAsyncQueue(node);
		}

		semaphora.post();
		return true;
	}

	bool asyncWrite(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemCallback> & cb, void * ptr, uintmax_t limit, uintmax_t offset, uintmax_t size) {
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h.fileHandle);
			if (it == fsHandleMap.end()) {
				return false;
			}
			h.asyncHandle = asyncHandleIndex;
			asyncHandleIndex++;
			h.status = AsyncStatus::WRITE;

			FS_AsyncNode node;
			node.status = AsyncStatus::WRITE;
			node.handle = h.fileHandle;
			node.asyncHandle = h.asyncHandle;
			node.data = (unsigned char *)ptr;
			node.dataLimit = limit;
			node.handle_st = &it->second;
			node.fileStart = offset;
			node.filePos = offset;
			node.dataSize = size;
			node.dataPos = 0;
			node.callback = cb;
			postToAsyncQueue(node);
		}

		semaphora.post();
		return true;
	}

	bool abortAsyncOperation(FS_AsyncHandle_ST & h) {
		std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h.fileHandle);
		if (it == fsHandleMap.end()) {
			return false;
		}
		{
			boost::lock_guard<boost::shared_mutex> lg(lock);
			FS_AsyncNode node;
			node.status = AsyncStatus::WRITE;
			node.handle = h.fileHandle;
			node.asyncHandle = h.asyncHandle;
			node.handle_st = &it->second;
			postToAsyncQueue(node, QueueOperation::PUSH_FRONT);
		}
		semaphora.post();
	}
};

