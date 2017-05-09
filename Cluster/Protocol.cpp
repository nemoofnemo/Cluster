#include "Protocol.h"

string & Protocol::operator[](const string & key)
{
	return dataMap[key];
}

string & Protocol::get(const string & key)
{
	return dataMap[key];
}

void Protocol::set(const string & key, const string & value)
{
	dataMap[key] = value;
}

bool Protocol::isExist(const string & key)
{
	std::map<string, string>::iterator it = dataMap.find(key);
	if (it == dataMap.end())
		return false;
	else
		return true;
}

void Protocol::del(const string & key)
{
	std::map<string, string>::iterator it = dataMap.find(key);
	if (it != dataMap.end())
		dataMap.erase(it);
}

bool Protocol::match(void * data, size_t size)
{
	dataMap.clear();
	if (data == NULL || size <= 0) {
		return false;
	}
	nemo::ByteBuffer buf(size+1);
	buf.memcpy(data, size);
	buf[size + 1] = '\0';

	const char * ptr = buf.getData();
	const char * last = NULL;
	boost::cmatch w;
	boost::regex r(R"((\w*?)\s\s)");
	boost::cmatch w2;
	boost::regex r2(R"((\w+):(\w+))");
	bool blankLine = false;

	while (boost::regex_search(ptr, w, r)) {
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
		ptr = w[0].second;
	}
	
	if (!blankLine) {
		dataMap.clear();
		return false;
	}

	if (ptr - buf.getData() != 0) {

	}

	return true;
}

void Protocol::allocateContent(size_t size)
{
	if (content != NULL) {
		content.reset();
	}
	else {
		content = boost::shared_ptr<nemo::ByteBuffer>(new nemo::ByteBuffer(size));
	}
}

boost::shared_ptr<nemo::ByteBuffer> Protocol::getContent(void)
{
	return content;
}
