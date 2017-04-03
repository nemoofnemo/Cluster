#include "thread_pool.h"
#include <iostream>

struct fc {
	void operator()(void) {
		puts("11111111111");
	}
};

void fuck(void) {
	puts("2222");
}

void shit(int a, int b) {
	printf("%d %d\n", a, b);
}

class ff {
public:
	virtual void show(int a) {
		printf("%d\n", a);
	}
};

class ff2 : public ff {
public:
	void show(int a) {
		printf("fucccck");
	}
};

int main(void) {
	boost::shared_ptr<ff> ptr = boost::shared_ptr<ff>(new ff2);
	ptr->show(1);
	return 0;

}