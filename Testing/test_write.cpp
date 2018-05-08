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
	create_test_file("test");

	//for testing mid size file
	//create_mid_file("test");
	//assume mount to root directory
	f_mount("/","test");

	//test f_write small file
	fd = f_open("/test.txt","a");
	char* test_text = "It is the choice of Steins Gate!\n";
	//test f_write here
	//f_write(test_text, strlen(test_text),1,fd);
	printf("I write %d by f_write\n",strlen(test_text));
	//get the inode region
	fp = fopen("test","r");
	fseek(fp, 0, SEEK_END);
  	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	file_buffer = (char*) malloc(size);
	fread(file_buffer, size ,1,fp);
  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
	print_superblock(sp);
	inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    print_inode_region(sp,file_buffer);
	//use f_seek to go back
	f_seek(fd,0,SEEK_SET);

	char* test_block = (char*)(malloc(BLOCK_SIZE));
	int out_size = f_read(test_block,BLOCK_SIZE,1,fd);
	if(out_size > 0)
		printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
	else
		printf("something wrong about f_read or write or seek!\n");
	return 0;
}