#include <iostream>
#include <string>
#include <list>

#define _WIN32_WINNT  0x0501
#include <boost\asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;

class st {
public:
	int a;
	int b;

	st() {
		a = 1;
		b = 2;
		puts("cccc");
	}

	~st() {
		puts("dddd");
	}

	void show() {
		puts("show");
	}
};

int main(void) {
	list<boost::shared_ptr<st>> l;
	for (int i = 0; i < 3; ++i) {
		l.push_back(boost::shared_ptr<st>(new st));
	}
	puts("111");
	l.pop_back();
	puts("2222");
	return 0;
}