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

	int operator[](int i) {
		return 0;
	}
};

int binFind(int * a, int length, int val) {
	int mid = (length - 1) / 2;
	int start = 0;
	int end = length - 1;
	while (end - 1 > start) {
		if (a[mid] == val) {
			return mid;
		}
		else if (val > a[mid]) {
			start = mid + 1;
		}
		else {
			end = mid - 1;
		}
	}
	return -1;
}

int main(void) {
	char * data = new char[256];
	A * p = new(data) A;
	cout << (void*)data << ' ' << p << ' ' << sizeof(A) << endl;
	p->~A();
	delete[] data;
	p->operator[](1);
	(*p)[1];
	return 0;
}