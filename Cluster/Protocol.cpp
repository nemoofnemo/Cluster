#include "Protocol.h"

string & nemo::Protocol::operator[](const string & key)
{
	return dataMap[key];
}

string & nemo::Protocol::get(const string & key)
{
	return dataMap[key];
}

void nemo::Protocol::set(const string & key, const string & value)
{
	dataMap[key] = value;
}

bool nemo::Protocol::isExist(const string & key)
{
	std::map<string, string>::iterator it = dataMap.find(key);
	if (it == dataMap.end())
		return false;
	else
		return true;
}

void nemo::Protocol::del(const string & key)
{
	std::map<string, string>::iterator it = dataMap.find(key);
	if (it != dataMap.end())
		dataMap.erase(it);
}

bool nemo::Protocol::match(const void * data, size_t size)
{
	dataMap.clear();
	if (data == NULL || size <= 0) {
		return false;
	}
	nemo::ByteBuffer buf(size+1);
	buf.memcpy(data, size);
	buf[size] = 0;

	const char * ptr = buf.getData();
	boost::cmatch w;
	boost::regex r(R"((.*?)\s\s)");
	boost::cmatch w2;
	boost::regex r2(R"((\w+):(\w+))");
	bool blankLine = false;
	//puts(ptr);
	//puts("--------------");
	while (boost::regex_search(ptr, w, r)) {
		ptr = w[0].second;
		if (w[1].length() == 0) {
			blankLine = true;
			break;
		}
		
		if (boost::regex_match(w[1].first, w[1].second, w2, r2)) {
			dataMap.insert(std::pair<string, string>(w2[1],w2[2]));
		}
		else {
			dataMap.clear();
			return false;
		}
	}
	
	if (dataMap.size() == 0) {
		return false;
	}

	if (!blankLine) {
		dataMap.clear();
		return false;
	}

	//end of data?
	size_t cnt = ptr - buf.getData();
	if (cnt != size) {
		std::map<string, string>::iterator temp = dataMap.find("ContentLength");
		if (temp == dataMap.end()) {
			dataMap.clear();
			return false;
		}
		int contLen = atoi(temp->second.c_str());
		if (contLen != size - cnt) {
			dataMap.clear();
			return false;
		}
		content = boost::shared_ptr<nemo::ByteBuffer>(new nemo::ByteBuffer(contLen));
		content->memcpy((void*)ptr, contLen);
	}

	return true;
}

void nemo::Protocol::allocateContent(size_t size)
{
	if (content != NULL) {
		content.reset();
	}
	else {
		content = boost::shared_ptr<nemo::ByteBuffer>(new nemo::ByteBuffer(size));
	}
}

boost::shared_ptr<nemo::ByteBuffer> nemo::Protocol::getContent(void)
{
	return content;
}
