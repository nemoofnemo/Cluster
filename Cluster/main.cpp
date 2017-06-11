#include "ClusterInclude.h"
#include "Protocol.h"
using namespace std;
using namespace nemo;

int main(void) {
	string str = "a:1\r\nContentLength:6\r\nb:2\r\n\r\nsbsbsb";
	Protocol p;
	p.match(str.c_str(), str.size());
	string xx = p["kk"];
	string xx2 = p["a"];
	return 0;
}