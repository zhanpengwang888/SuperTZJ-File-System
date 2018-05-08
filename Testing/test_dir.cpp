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
	format_default_size("test");
	f_mount("/", "test");
	create_test_file("test");

	// Test f_mkdir:
	int make_a_directory = f_mkdir("/test_dir", RDONLY + EXEONLY + WRONLY);
	cout << "[f_mkdir] The returned value is " << make_a_directory << endl;
	
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
	inode* my_dir = next_inode(root);
    print_inode_region(sp,file_buffer);
	print_directory(sp, root, file_buffer); // print the directory.
	print_directory(sp,my_dir,file_buffer);
	
	return 0;
}