#include "FileSystem.h"

using namespace std;

int main(void) {
	/*boost::filesystem::recursive_directory_iterator it(".");
	boost::filesystem::recursive_directory_iterator end;
	while (it != end) {		
		cout << (*it).path().string();
		if (!boost::filesystem::is_directory(*it)) {
			cout << boost::filesystem::file_size(*it);
		}
		
		cout << endl;
		it++;
	}*/
	fstream f;
	f.open("test.txt", ios::in | ios::binary);
	f.seekg(10000, ios::beg);
	if (f.good()) {
		puts("1111");
	}
	f.write("1", 1);
	if (f.eof()) {
		puts("222");
	}
	cout << f.tellg() << endl;
	f.close();
	return 0;
}