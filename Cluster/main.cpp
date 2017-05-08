#include "ClusterInclude.h"

using namespace std;

void split(std::string& s, std::string& delim, std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + 1;
		index = s.find_first_of(delim, last);
	}
	if (index - last>0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}

int main(void) {
	char * ptr = "hello\r\nddfdd\r\ndsss\r\neeeee\r\ndddddddddee";
	string str(ptr, 31);
	std::vector< std::string > vec;
	split(str, string("\r\n"), &vec);
	auto it = vec.begin();
	auto end = vec.end();
	while (it != end) {
		cout << *it << endl;
		++it;
	}

	return 0;
}