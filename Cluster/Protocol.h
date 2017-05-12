#pragma once
#include "ClusterInclude.h"
#include "ClusterUtil.h"

/*
protocol format:
ket-value table.

key1:value1\r\n
key2:value2\r\n
.....
keyn:valuen\r\n
[ContentLength:2333]
\r\n

[optional appendent content(length in bytes)]:
data length must be specified by contentlength.
*/

using boost::regex;
using std::map;
using std::string;

namespace nemo {
	class Protocol;
}

class nemo::Protocol {
private:
	map<string, string> dataMap;
	boost::shared_ptr<nemo::ByteBuffer> content;

public:
	Protocol() {

	}

	Protocol(Protocol & p) {
		dataMap = p.dataMap;
		content = boost::shared_ptr<nemo::ByteBuffer>(new nemo::ByteBuffer(*(p.content)));
	}

	Protocol & operator=(Protocol & p) {
		dataMap = p.dataMap;
		content = boost::shared_ptr<nemo::ByteBuffer>(new nemo::ByteBuffer(*(p.content)));
	}

	~Protocol() {
		
	}

	string & operator[](const string & key);

	string & get(const string & key);

	void set(const string & key, const string & value);

	bool isExist(const string & key);

	void del(const string & key);

	bool match(const void * data, size_t size);

	void allocateContent(size_t size);

	void setContentLength(size_t size);

	boost::shared_ptr<nemo::ByteBuffer> getContent(void);
};