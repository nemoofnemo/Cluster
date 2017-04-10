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
	if (boost::shared_ptr<int>() == NULL) {
		puts("1111");
	}
	return 0;
}