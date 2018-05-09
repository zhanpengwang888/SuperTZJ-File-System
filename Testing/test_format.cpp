#include "test_util.h"
#include "../fs.h"

using namespace std;
int main() {
	string name = "test";
	cout << "I am here" <<endl;
	//format_deafult_size(name);
	format_with_given_size(name, DEFAULT_SIZE * 2);
	f_mount("/","test");
	FILE* fp_r = fopen("test", "r");
	if(fp_r == NULL)
		perror("Error");
	//cout << "I am here" <<endl;
	char* buffer = (char*) malloc(DEFAULT_SIZE);
	int t_size = fread(buffer,DEFAULT_SIZE,1,fp_r);
	// if(t_size <= 0)
	// 	cout << "There are some problems" << endl;
	// else
	// 	cout << buffer << endl;
	Superblock* sp = (Superblock*)(buffer + BOOT_SIZE);
	cout << "??????" <<endl;
	print_superblock(sp);
	print_inode_region(sp,buffer);
	inode* root = (inode*)(buffer + BOOT_SIZE * 2);
	print_directory(sp,root,buffer);
	return TRUE;
}

