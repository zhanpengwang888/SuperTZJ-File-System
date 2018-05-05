#include "test_util.h"
#include "../format.cpp"

using namespace std;
int main() {
	char* name = "test";
	format_default_size(name);
	FILE* fp_r = fopen("test", "r");
	char* buffer = (char*) malloc(DEFAULT_SIZE);
	int t_size = fread(buffer,DEFAULT_SIZE,1,fp_r);
	if(t_size <= 0)
		cout << "There are some problems" << endl;
	else
		cout << buffer << endl;
	return TRUE;
}

