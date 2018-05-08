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
	printf("Testing program printing !!! $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	format_default_size("test");

	//for testing small file
	//create_test_file("test");

	//for testing mid size file
	create_mid_file("test");
	//assume mount to root directory
	f_mount("/","test");

	//test f_write small file
	fd = f_open("/test.txt","r");
	//get the inode region
	fp = fopen("test","r");
	f_seek(fd,5000,"SEEK_SET");
	fseek(fp, 0, SEEK_END);
  	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	file_buffer = (char*) malloc(size);
	fread(file_buffer, size ,1,fp);
  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
	print_superblock(sp);
	inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    //print_inode_region(sp,file_buffer);
    //go to the block to see what is in there
    // char* t_block = (char*) malloc(BLOCK_SIZE);
    // int data_address = inode_end + 12 * BLOCK_SIZE;
    // fseek(fp,data_address,SEEK_SET);
    // int test_size = fread(t_block,BLOCK_SIZE,1,fp);
    // if(test_size > 0)
    // 	printf("the content of test_block is %s\n",t_block);
	//use f_seek to go back
	//we should have pre define
	//f_seek(fd,10,"SEEK_SET");
	char* test_block = (char*)(malloc(BLOCK_SIZE));
	int out_size = f_read(test_block,BLOCK_SIZE,1,fd);
	if(out_size > 0)
		printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
	else
		printf("something wrong about f_read or write or seek!\n");
	return 0;
}