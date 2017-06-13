#include "FileSystem.h"

using namespace std;
using namespace nemo;

class cb : public FileSystemIO::FileSystemIOCallback {
public:
	void run(const FileSystemIO::FS_AsyncHandle_ST & ast, FileSystemIO::ErrorCode e, void * data, uintmax_t count) {
		cout << boost::this_thread::get_id() << ' ' << count << endl;
		for (int i = 0; i < count; ++i) {
			putchar(*(((char*)data) + i));
		}
		cout << endl;

		if(e != FileSystemIO::ErrorCode::PENDING)
			delete[] (char *)data;
	}
};

void test(void) {
	//char * temp = new char[256];
	FileSystemIO fs;
	fs.init();
	FileSystemIO::FS_Handle h = fs.createFileSystemHandle(boost::filesystem::path("test.txt"));
	FileSystemIO::FS_Handle h2 = fs.createFileSystemHandle(boost::filesystem::path("test2.txt"));
	
	FileSystemIO::FS_AsyncHandle_ST ah = fs.createAsyncHandleST(h);
	fs.asyncRead(ah, boost::shared_ptr<cb>(new cb), new char[256], 256, 0, 6);
	ah = fs.createAsyncHandleST(h2);
	fs.asyncRead(ah, boost::shared_ptr<cb>(new cb), new char[256], 256, 0, 6);

	ah = fs.createAsyncHandleST(h);
	fs.asyncRead(ah, boost::shared_ptr<cb>(new cb), new char[256], 256, 0, 6);
	ah = fs.createAsyncHandleST(h2);
	fs.asyncRead(ah, boost::shared_ptr<cb>(new cb), new char[256], 256, 0, 6);

	fs.run();
	boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(4));
	fs.stop();
}

void test2(void) {
	FileSystemIO fs;
	fs.init();
	FileSystemIO::FS_Handle h = fs.createFileSystemHandle(boost::filesystem::path("test.txt"));
	FileSystemIO::FS_AsyncHandle_ST ah = fs.createAsyncHandleST(h);
	fs.asyncRead(ah, boost::shared_ptr<cb>(new cb), new char[20], 20, 0, 6);
	fs.run();
	boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(4));
	fs.stop();
}

int main2(void) {
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
	//test();
	test2();
	return 0;
}