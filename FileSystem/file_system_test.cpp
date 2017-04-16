#include "FileSystem.h"

using namespace std;

class cb : public FileSystem::FileSystemCallback {
public:
	void run(const FileSystem::FS_AsyncHandle_ST & ast, FileSystem::ErrorCode e, void * data, uintmax_t count) {
		cout << count << endl;
		for (int i = 0; i < count; ++i) {
			putchar(*(((char*)data) + i));
		}
	}
};

void test(void) {
	FileSystem fs;
	fs.init();
	FileSystem::FS_Handle h = fs.createFileSystemHandle(boost::filesystem::path("test2.txt"));
	FileSystem::FS_AsyncHandle_ST ah = fs.createAsyncHandleST(h);
	char * temp = new char[256];
	//fs.asyncRead(ah, boost::shared_ptr<cb>(new cb), temp, 256, 999, 6);
	fs.asyncAppendWrite(ah, boost::shared_ptr<cb>(new cb), "filesystem", 10);
	fs.debugRun();
}

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
	/*fstream f;
	f.open("test2.txt", ios::out | ios::binary | ios::ate | ios::app);
	char temp[100] = { 'a' };
	f.write(temp, 5);
	f.close();*/
	test();
	return 0;
}