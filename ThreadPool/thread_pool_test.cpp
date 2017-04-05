#include "ThreadPool.h"
#include <iostream>

using namespace std;

class cb : public nemo::ThreadPoolCallback {
public:
	void run(void) {
		cout << "run in " << boost::this_thread::get_id() << endl;
	}
};

int main(void) {
	nemo::ThreadPool tp;
	boost::shared_ptr<nemo::ThreadPoolCallback> ptr = boost::shared_ptr<nemo::ThreadPoolCallback>(new cb);
	tp.run();

	for (int i = 0; i < 10; ++i) {
		tp.addEvent(ptr);
	}

	boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(3));
	tp.stop();
	return 0;
}