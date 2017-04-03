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

struct kk {
	int x = 10;
};

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

	void show(kk k) {
		puts("show");
		cout << k.x << endl;
	}

	void run() {
		kk k;
		bind(&st::show, this, k);
	}
};

int main(void) {
	st s;
	s.run();
	return 0;
}