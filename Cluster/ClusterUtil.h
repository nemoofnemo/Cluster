#pragma once
#include "ClusterInclude.h"

namespace nemo {
	class ByteBuffer;
}

class nemo::ByteBuffer {
private:
	char * data;
	size_t size;
public:
	ByteBuffer() {

	}

	ByteBuffer(ByteBuffer & b) {


	}

	ByteBuffer & operator=(ByteBuffer & b) {

	}

	~ByteBuffer() {

	}

	void realocate(size_t n) {

	}

	char & operator[](const int & index) {

	}

};