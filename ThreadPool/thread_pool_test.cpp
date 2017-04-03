#include "thread_pool.h"
#include <iostream>
using namespace std;

class CB : public nemo::ThreadPoolCallback {
public:
	virtual void run(void) {
		cout << "run in" << boost::this_thread::get_id() << endl;
	}
};

int main(void) {
	nemo::ThreadPool tp;
	boost::shared_ptr<nemo::ThreadPoolCallback> ptr = boost::shared_ptr<nemo::ThreadPoolCallback>(new CB);
	tp.run();
	for (int i = 0; i < 10; ++i) {
		tp.addEvent(ptr);
	}
	
	boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(10));
	tp.stop();
	return 0;
}