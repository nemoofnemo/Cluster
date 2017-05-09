#pragma once
#include "ClusterInclude.h"

namespace nemo {
	enum Default{BLOCK_SIZE=4096};
	class ByteBuffer;

	void split(std::string & str, const std::string& sp, std::vector< std::string >* ret);
}

class nemo::ByteBuffer {
private:
	char * m_data;
	size_t m_size;

public:
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

	void memcpy(const void * src, size_t cnt) {
		if (cnt == 0 || src == NULL) {
			return;
		}
		if (cnt > m_size) {
			delete[] m_data;
			m_data = new char[cnt];
			m_size = cnt;
		}
		memcpy_s(m_data, m_size, src, cnt);
	}

	const char * getData(void) {
		return m_data;
	}

	const size_t size(void) {
		return m_size;
	}
};