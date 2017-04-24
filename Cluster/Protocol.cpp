#include "Protocol.h"

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

class Protocol {
private:
	map<string, string> dataMap;
public:
	Protocol() {

	}

	Protocol(Protocol & p) {

	}

	Protocol & operator=(Protocol & p) {

	}

	~Protocol() {

	}

	const string & operator[](const string & key);

	const string & get(const string & key);

	const string & set(const string & key, const string & value);

	bool del(const string & key);

	void allocateContent(size_t size);

};