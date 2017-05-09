#include "ClusterUtil.h"

void nemo::split(std::string & str, const std::string& sp, std::vector< std::string >* ret){
	size_t last = 0;
	size_t pos = str.find(sp);
	while (pos != std::string::npos) {
		ret->push_back(str.substr(last, pos - last));
		last = pos + sp.size();
		pos = str.find(sp, last);
	}
	if (pos - last > 0) {
		ret->push_back(str.substr(last, pos - last));
	}
}