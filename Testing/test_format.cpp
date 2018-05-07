#include "test_util.h"
#include "../fs.h"

using namespace std;
int main() {
	string name = "test";
	format_default_size(name);
	FILE* fp_r = fopen("test", "r");
	if(fp_r == NULL)
		perror("Error");
	char* buffer = (char*) malloc(DEFAULT_SIZE);
	int t_size = fread(buffer,DEFAULT_SIZE,1,fp_r);
	// if(t_size <= 0)
	// 	cout << "There are some problems" << endl;
	// else
	// 	cout << buffer << endl;
	Superblock* sp = (Superblock*)(buffer + BOOT_SIZE);
	print_superblock(sp);
	print_inode_region(sp,buffer);
	return TRUE;
}

