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
	if (data == NULL || size <= 0) {
		return;
	}
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
