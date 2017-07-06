#include <iostream>
#include <string>
#include <list>
#include <queue>

//#define _WIN32_WINNT  0x0501
//#include <boost/asio.hpp>
//#include <boost/thread.hpp>
//#include <boost/bind.hpp>
//#include <boost/function.hpp>

using namespace std;
//using namespace boost;
//using namespace boost::asio;
//#include "vld.h"

class A {
public:
	int a;
	A() {
		cout << "con" << endl;
	}

	~A() {
		cout << "dec" << endl;
	}
};


int main(void) {
	char * data = new char[256];
	A * p = new(data) A;
	cout << (void*)data << ' ' << p << ' ' << sizeof(A) << endl;
	p->~A();
	delete[] data;
	return 0;
}