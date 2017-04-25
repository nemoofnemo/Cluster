#pragma once
#include "ClusterInclude.h"

namespace nemo {
	enum Default{BLOCK_SIZE=4096};
	class ByteBuffer;
}

class nemo::ByteBuffer {
public:
	char * m_data;
	size_t m_size;

	ByteBuffer() {
		m_size = Default::BLOCK_SIZE;
		m_data = new char[m_size];
	}

	ByteBuffer(size_t size) {
		if (size == 0) {
			m_size = Default::BLOCK_SIZE;
		}
		m_size = size;
		m_data = new char[m_size];
	}

	ByteBuffer(ByteBuffer & b) {
		m_size = b.m_size;
		m_data = new char[m_size];
		memcpy_s(m_data, m_size, b.m_data, m_size);
	}

	ByteBuffer & operator=(ByteBuffer & b) {
		m_size = b.m_size;
		m_data = new char[m_size];
		memcpy_s(m_data, m_size, b.m_data, m_size);
		return *this;
	}

	~ByteBuffer() {
		delete[] m_data;
	}

	void reallocate(size_t size) {
		if (size == 0) {
			m_size = Default::BLOCK_SIZE;
		}
		
		char * temp = new char[size];
		memcpy_s(temp, size, m_data, m_size);
		delete[] m_data;
		m_data = temp;
		m_size = size;
	}

	char & operator[](const size_t & index) {
		if (index > m_size) {
			throw std::exception("out of bound");
		}
		return m_data[index];
	}

};