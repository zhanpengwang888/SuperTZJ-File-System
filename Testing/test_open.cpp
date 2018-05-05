#include "test_util.h"
#include "../fs.h"
using namespace std;
int main() {
	f_open("/usr/doc/abc.txt", "r");
	return 0;
}