#include "thread_pool.h"
#include <iostream>
using namespace std;

class CB : public nemo::ThreadPoolCallback {
public:
	void operator()(void) {
		cout << "run in" << boost::this_thread::get_id() << endl;
	}
};

int main(void) {
	
	return 0;
}