#include "FileSystem.h"

using namespace std;

int main(void) {
	boost::filesystem::recursive_directory_iterator it(".");
	boost::filesystem::recursive_directory_iterator end;
	while (it != end) {		
		cout << (*it).path().string();
		if (!boost::filesystem::is_directory(*it)) {
			cout << boost::filesystem::file_size(*it);
		}
		
		cout << endl;
		it++;
	}
	
	return 0;
}