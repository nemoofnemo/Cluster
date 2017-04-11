#include "ThreadPool.h"
#include <iostream>
#include <cstdlib>

using namespace std;

class cb : public nemo::ThreadPoolCallback {
public:
	void run(void) {
		cout << "run in " << boost::this_thread::get_id() << endl;
	}
};

int main(void) {
	nemo::ThreadPool tp;
	tp.run();
	for (int i = 0; i < 10; ++i) {
		tp.postTask(boost::shared_ptr<nemo::ThreadPoolCallback>(new cb));
	}
	puts("post done");
	boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(3));
	puts("stop");
	tp.stop();
	return 0;
}