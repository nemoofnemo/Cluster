#include "ClusterInclude.h"

using namespace std;

void split(std::string& s, std::string& delim, std::vector< std::string >* ret)
{
	size_t last = 0;
	size_t index = s.find_first_of(delim, last);
	while (index != std::string::npos)
	{
		ret->push_back(s.substr(last, index - last));
		last = index + delim.size();
		index = s.find_first_of(delim, last);
	}
	if (index - last>0)
	{
		ret->push_back(s.substr(last, index - last));
	}
}

void split2(std::string & str, const std::string& sp, std::vector< std::string >* ret) {
	size_t last = 0;
	size_t pos = str.find(sp);
	while (pos != std::string::npos) {
		ret->push_back(str.substr(last, pos - last));
		last = pos + sp.size();
		pos = str.find(sp, last);
	}
	if (last < str.size()) {
		ret->push_back(str.substr(last, pos - last));
	}
}

int main(void) {
	char * ptr = "hello\r\nddfdd\r\ndsss\r\neeeee\r\n\r\nlll";
	string str(ptr, 31);
	/*std::vector< std::string > vec;
	split2(str, string("\r\n"), &vec);
	auto it = vec.begin();
	auto end = vec.end();
	while (it != end) {
		cout << *it << endl;
		++it;
	}*/

	/*boost::regex r(R"(\w*?\s\s)");
	boost::smatch what;
	std::string::const_iterator it = str.begin();
	std::string::const_iterator end = str.end();
	std::vector< std::string > vec;
	while (boost::regex_search(it, end, what, r)) {
		vec.push_back(what[0]);
		std::cout << what[0] << std::endl;
		it = what[0].second;
	}*/
	
	const char * ptr1 = ptr;
	boost::cmatch w;
	boost::regex r(R"((\w*?)\s\s)");
	std::vector< std::string > vec;
	while (boost::regex_search(ptr1, w, r)) {
		vec.push_back(w[0]);
		boost::cmatch w2;
		if(boost::regex_match(w[1].first, w[1].second, w2, boost::regex(R"(\w*?)")))
			std::cout << w[1] << std::endl;
		ptr1 = w[0].second;
	}

	return 0;
}