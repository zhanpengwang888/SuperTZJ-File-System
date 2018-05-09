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

int main(int argc, char** argv) {
	format_default_size("test");
	f_mount("/", "test");
	int make_a_directory;
	//create_test_file("test");

	// Test f_mkdir: create one directory
	make_a_directory = f_mkdir("/test_dir", RDONLY + EXEONLY + WRONLY);
	cout << "[f_mkdir] The returned value is " << make_a_directory << endl;
	//Test: create a directory under a directory
	make_a_directory = f_mkdir("/test_dir/nested_test", RDONLY + EXEONLY + WRONLY);
	cout << "[f_mkdir] The returned value is " << make_a_directory << endl;
	
	// Test Duplicate Directory.
	make_a_directory = f_mkdir("/test_dir/nested_test", RDONLY + EXEONLY + WRONLY);
	if (make_a_directory != FAIL) {
		cout << "Duplicate directory tests fails." << endl;
		cout << "[f_mkdir] The returned value is " << make_a_directory << endl;
	}

	// Test: 16 directories that fit in a single data block
	/*string directory_name = "/test_dir";
	string temp_string = directory_name;
	for(int i = 0; i < BLOCK_SIZE/sizeof(directory_entry); i++) {
		directory_name += to_string(i);
		make_a_directory = f_mkdir(directory_name, RDONLY + EXEONLY + WRONLY);
		directory_name = temp_string;
		if (make_a_directory == FAIL)
			return FAIL;
	}*/

	// Test: write into first indirect data blocks
	/*string directory_name = "/test_dir";
	string temp_string = directory_name;
	for(int i = 0; i < N_DBLOCKS * BLOCK_SIZE/sizeof(directory_entry); i++) {
		directory_name += to_string(i);
		make_a_directory = f_mkdir(directory_name, RDONLY + EXEONLY + WRONLY);
		directory_name = temp_string;
		if (make_a_directory == FAIL)
			return FAIL;
	}*/
	
	/*
	// write a test.txt file into the test_dir directory.
	fd = f_open("/test_dir/test.txt", "w");
	char* test_text = (char*)malloc(BLOCK_SIZE * 10);
	for(int i = 0; i < 10; i ++) {
		strcat(test_text,junk_c);
	}
	printf("the length of test_text is %d\n",strlen(test_text));
	int w_size = f_write(test_text, strlen(test_text),1,fd);
	printf("I write %d by f_write\n",w_size);
	f_close(fd);

	fd = f_open("/test_dir/test.txt","r");
	char* test_block = (char*)(malloc(BLOCK_SIZE*10));
	int out_size = f_read(test_block,BLOCK_SIZE*10,1,fd);
	if(out_size > 0)
		printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
	else
		printf("something wrong about f_read or write or seek!\n");
	f_close(fd);

	// write a test.txt file into the nested_test directory.
	fd = f_open("/test_dir/nested_test/test.txt", "w");
	test_text = (char*)malloc(BLOCK_SIZE * 10);
	for(int i = 0; i < 10; i ++) {
		strcat(test_text,junk_c);
	}
	printf("the length of test_text is %d\n",strlen(test_text));
	w_size = f_write(test_text, strlen(test_text),1,fd);
	printf("I write %d by f_write\n",w_size);
	f_close(fd);

	fd = f_open("/test_dir/nested_test/test.txt","r");
	test_block = (char*)(malloc(BLOCK_SIZE*10));
	out_size = f_read(test_block,BLOCK_SIZE*10,1,fd);
	if(out_size > 0)
		printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
	else
		printf("something wrong about f_read or write or seek!\n");
	f_close(fd);
	*/

	// Test open a directory                                                                                                                                                
        cout << "Testing f_opendir" << endl;
        int directory_open = f_opendir("/");
        if (directory_open == FAIL) {
                cout << "[f_opendir] Fails" << endl;
                return FAIL;
        }
        printf("\n");
        // Test read a directory                                                                                                                                                
        cout << "Testing f_readdir" << endl;
        directory_entry* entry;
        while ((entry = f_readdir(directory_open)) != NULL) {
          cout << "Entry: "<< entry->file_name << endl;
        }
        printf("\n");
        // Test close a directory                                                                                                                                               
        cout << "Testing f_closedir" << endl;
        directory_open = f_closedir(directory_open);
	
	fp = fopen("test","r");
	fseek(fp, 0, SEEK_END);
  	size_t size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	file_buffer = (char*) malloc(size);
	fread(file_buffer, size ,1,fp);
  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
	inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
	inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
	inode_num = (inode_end - inode_start) / INODE_SIZE;
	print_superblock(sp);
	inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
	inode* my_dir = next_inode(root);
	inode* nested_dir = next_inode(my_dir);
    print_inode_region(sp,file_buffer);
	print_directory(sp, root, file_buffer); // print the directory.
	print_directory(sp,my_dir,file_buffer);
	print_directory(sp, nested_dir, file_buffer);
	return 0;
}
