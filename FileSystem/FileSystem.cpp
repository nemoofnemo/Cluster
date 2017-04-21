#include "FileSystem.h"

using nemo::FileSystemIO;

void nemo::FileSystemIO::postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op) {
	if (op == QueueOperation::PUSH_BACK) {
		asyncQueue.push_back(node);
	}
	else {
		asyncQueue.push_front(node);
	}
}

void nemo::FileSystemIO::_postToAsyncQueue(const FS_AsyncNode & node, QueueOperation op) {
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

void nemo::FileSystemIO::doRead(FS_AsyncNode & node, const int & index) {
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
	target = (target > node.dataLimit) ? node.dataLimit : target;

	file.read((char *)node.data, target);
	uintmax_t cnt = file.gcount();
	node.dataPos += cnt;

	if (file.good() && cnt > 0) {
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

void nemo::FileSystemIO::doReadAll(FS_AsyncNode & node, const int & index) {
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
	target = (target > node.dataLimit) ? node.dataLimit : target;

	file.read((char *)node.data, target);
	uintmax_t cnt = file.gcount();
	node.dataPos += cnt;

	if (file.good() && cnt > 0) {
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

		if (cnt == node.dataLimit) {
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

void nemo::FileSystemIO::doWrite(FS_AsyncNode & node, const int & index) {
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
	target = (target > node.dataLimit) ? node.dataLimit : target;

	file.write((char *)node.data, target);
	file.flush();
	/*uintmax_t cnt = file.gcount();*/
	uintmax_t cnt = target;
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

void nemo::FileSystemIO::doAppendWrite(FS_AsyncNode & node, const int & index) {
	std::fstream file;
	file.open(node.handle_st->fullPath.string(), std::ios::binary | std::ios::out | std::ios::ate | std::ios::app);
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
	target = (target > node.dataLimit) ? node.dataLimit : target;

	file.seekp(0, std::ios::end);
	file.write((char *)node.data, target);
	file.flush();
	//WARNING: cnt = target
	uintmax_t cnt = target;
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

void nemo::FileSystemIO::doAbort(FS_AsyncNode & node, const int & index) {
	if (node.callback != NULL) {
		FS_AsyncHandle_ST st;
		st.asyncHandle = node.asyncHandle;
		st.fileHandle = node.handle;
		st.status = node.status;
		node.callback->run(st, ErrorCode::DONE, node.data, 0);
	}
}

void nemo::FileSystemIO::doUnknownError(FS_AsyncNode & node, const int & index) {
	if (node.callback != NULL) {
		FS_AsyncHandle_ST st;
		st.asyncHandle = node.asyncHandle;
		st.fileHandle = node.handle;
		st.status = node.status;
		node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, 0);
	}
}

void nemo::FileSystemIO::doExit(FS_AsyncNode & node, const int & index) {
	if (node.callback != NULL) {
		FS_AsyncHandle_ST st;
		st.asyncHandle = node.asyncHandle;
		st.fileHandle = node.handle;
		st.status = node.status;
		node.callback->run(st, ErrorCode::UNKNOWN_ERROR, node.data, 0);
	}
}

void nemo::FileSystemIO::executeIO(FS_AsyncNode & node, const int & index) {
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

void nemo::FileSystemIO::workThread(int index) {
	printf("workthread:%d\n",index);
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
			
			//exit
			if (it->status == AsyncStatus::EXIT) {
				break;
			}

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
	return;
}

//config filesystem io
void nemo::FileSystemIO::init(void) {
	boost::lock_guard<boost::shared_mutex> lg(lock);
	handleIndex = 0;
	asyncHandleIndex = 0;
	blockSize = 4096;
	threadNum = 2;
	exitFlag = false;
}

void nemo::FileSystemIO::debugRun() {
	//blockSize = 1;
	FS_Thread_Proceing tp;
	processingVector.push_back(tp);
	workThread(0);
}

void nemo::FileSystemIO::run(void) {
	boost::lock_guard<boost::shared_mutex> lg(lock);
	for (int i = 0; i < threadNum; ++i) {
		FS_Thread_Proceing tp;
		processingVector.push_back(tp);
		threadList.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&FileSystemIO::workThread, this, i))));
	}
}

void nemo::FileSystemIO::stop(void) {
	{
		boost::lock_guard<boost::shared_mutex> lg(lock);
		for (int i = 0; i < threadNum; ++i) {
			FS_AsyncNode nd;
			nd.status = AsyncStatus::EXIT;
			postToAsyncQueue(nd);
			semaphora.post();
		}
		exitFlag = true;
	}
	std::list<boost::shared_ptr<boost::thread>>::iterator it = threadList.begin();
	std::list<boost::shared_ptr<boost::thread>>::iterator end = threadList.end();
	while (it != end) {
		(*it)->join();
		it++;
	}
	threadList.clear();
	processingVector.clear();
}

nemo::FileSystemIO::FS_Handle nemo::FileSystemIO::createFileSystemHandle(const boost::filesystem::path & p) {
	boost::lock_guard<boost::shared_mutex> lg(lock);
	FS_Handle ret = handleIndex;
	handleIndex++;
	FS_Handle_ST st;
	st.handle = ret;
	st.fullPath = p;
	fsHandleMap.insert(std::pair<FS_Handle, FS_Handle_ST>(ret, st));
	return ret;
}

void nemo::FileSystemIO::releaseFileSystemHandle(FS_Handle h) {
	if (exitFlag) {
		return;
	}
	{
		boost::lock_guard<boost::shared_mutex> lg(lock);
		std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h);
		if (it != fsHandleMap.end()) {
			fsHandleMap.erase(it);
		}
	}
}

nemo::FileSystemIO::FS_AsyncHandle_ST nemo::FileSystemIO::createAsyncHandleST(FS_Handle h) {
	nemo::FileSystemIO::FS_AsyncHandle_ST ret;
	ret.fileHandle = h;
	return ret;
}

bool nemo::FileSystemIO::asyncRead(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t limit, uintmax_t offset, uintmax_t size) {
	if (exitFlag || ptr == NULL || limit == 0 || size == 0) {
		return false;
	}

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

bool nemo::FileSystemIO::asyncReadAll(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t limit, uintmax_t size) {
	if (exitFlag || ptr == NULL || limit == 0 || size == 0) {
		return false;
	}

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

bool nemo::FileSystemIO::asyncAppendWrite(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t size) {
	if (exitFlag || ptr == NULL || size == 0) {
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

bool nemo::FileSystemIO::asyncWrite(FS_AsyncHandle_ST & h, const boost::shared_ptr<FileSystemIOCallback> & cb, void * ptr, uintmax_t limit, uintmax_t offset, uintmax_t size) {
	if (exitFlag || ptr == NULL || limit == 0 || size == 0) {
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

bool nemo::FileSystemIO::abortAsyncOperation(FS_AsyncHandle_ST & h) {
	if (exitFlag) {
		return false;
	}
	{
		boost::lock_guard<boost::shared_mutex> lg(lock);
		std::map<FS_Handle, FS_Handle_ST>::iterator it = fsHandleMap.find(h.fileHandle);
		if (it == fsHandleMap.end()) {
			return false;
		}
		FS_AsyncNode node;
		node.status = AsyncStatus::ABORT;
		node.handle = h.fileHandle;
		node.asyncHandle = h.asyncHandle;
		node.handle_st = &it->second;
		postToAsyncQueue(node, QueueOperation::PUSH_FRONT);
	}
	semaphora.post();
	return true;
}