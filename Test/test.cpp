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
	int mid = 0;
	int start = 0;
	int end = length - 1;
	while (end >= start) {
		mid = (start + end) / 2;
		if (a[mid] == val) {
			return mid;
		}
		else if (val > a[mid]) {
			start = mid + 1;
		}
		else {
			end = mid - 1;
		}
		cout << start << ' ' << end << endl;
	}
	return -1;
}

int main(void) {
	int arr[] = {1,2,3,4,5,6,7,8};
	cout << binFind(arr, 8, 8) << endl;
	return 0;
}