#include "test_util.h"
#include "../fs.h"
using namespace std;

int fd;
FILE* fp;
char* file_buffer;
Superblock *sp; // super block
int inode_start;
int inode_end;
int inode_num;
//file_node *open_file_table[MAX_OPEN_FILE];
inode *disk_inode_list[MAX_INODE_NUM];
string junk (512,'k');
const char* junk_c = junk.c_str();

int main() {
	//f_open("/usr/doc/abc.txt", "r");
	format_default_size("test");

	//for testing small file
	//create_test_file("test");

	//for testing mid size file
	//create_mid_file("test");

	//assume mount to root directory
	f_mount("/","test");

	/*
	//test f_remove small file
	printf("Testing program printing !!! $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	int test_fd = f_open("/test.txt","r");
	fp = fopen("test","r");
	fseek(fp, 0, SEEK_END);
  	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(file_buffer, size , 1,fp);
  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
  	print_inode_region(sp,file_buffer);
	print_superblock(sp);
	f_close(test_fd);
	f_remove("/test.txt");
	char* test_block = (char*) malloc(BLOCK_SIZE);
	int data_address = inode_end + 1 * BLOCK_SIZE;
	fseek(fp,data_address,SEEK_SET);
	fread(test_block,BLOCK_SIZE,1,fp);
	int* free_one = (int*) test_block;
	printf("the next free block is %d\n",free_one[0]);
	inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    //also need to print root directory
    print_directory(sp,root,file_buffer);
	//test the current inode region
	fseek(fp,0,SEEK_SET);
	fread(file_buffer, size , 1,fp);
	print_inode_region(sp,file_buffer);
	*/

	//test f_remove middle size file
	
	int test_fd = f_open("/test.txt","r");
	f_close(test_fd);
	f_remove("/test.txt");
	printf("Testing program printing !!! $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	char* test_block = (char*) malloc(BLOCK_SIZE);
	fp = fopen("test","r");
	for(int i = 1; i < 11; i ++) {
		int data_address = inode_end + i * BLOCK_SIZE;
		fseek(fp,data_address,SEEK_SET);
		fread(test_block,BLOCK_SIZE,1,fp);
		int* free_one = (int*) test_block;
		printf("the next free block is %d\n",free_one[0]);
	}
	fseek(fp, 0, SEEK_END);
  	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(file_buffer, size , 1,fp);
  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
	print_superblock(sp);
	inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    //also need to print root directory
    print_directory(sp,root,file_buffer);
    


	return 0;
}