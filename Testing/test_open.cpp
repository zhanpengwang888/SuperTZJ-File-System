#include "test_util.h"
#include "../fs.h"
using namespace std;

int fd;
FILE* fp;
char* file_buffer;


//get a disk image and modify to be test disk image
void create_test_file(char* name) {
	fp = fopen(name,"r+");
	if(fp == NULL)
		perror("Error\n");
	fseek(fp, 0, SEEK_END);
  	size_t size = ftell(fp);
  	fseek(fp, 0, SEEK_SET);
  	cout << "the size of file is " << size <<endl;
  	file_buffer = (char*)malloc(sizeof(char) * size + 1);
  	Superblock* sp = (Superblock*)(file_buffer + BOOT_SIZE);
	print_superblock(sp);
	int inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
    int inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
    //print root directory's entries
    inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    print_directory(root,file_buffer);

}

int main() {
	//f_open("/usr/doc/abc.txt", "r");
	format_default_size("test");
	create_test_file("test");
	return 0;
}