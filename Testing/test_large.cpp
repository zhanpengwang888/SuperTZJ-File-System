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
	/*
	//test f_write small file
	fd = f_open("/test.txt","a");
	char* test_text = "It is the choice of Steins Gate!\n";
	//char* test_text = (char*)malloc(BLOCK_SIZE * 10);
	//test f_write here
	int w_size = f_write(test_text, strlen(test_text),1,fd);
	printf("I write %d by f_write\n",w_size);
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
    //print_inode_region(sp,file_buffer);
    //go to the block to see what is in there
    char* t_block = (char*) malloc(BLOCK_SIZE);
    int data_address = inode_end + 12 * BLOCK_SIZE;
    fseek(fp,data_address,SEEK_SET);
    int test_size = fread(t_block,BLOCK_SIZE,1,fp);
    if(test_size > 0)
    	printf("the content of test_block is %s\n",t_block);
	//use f_seek to go back
	//we should have pre define
	//f_seek(fd,10,"SEEK_SET");
    f_close(fd);
    fd = f_open("/test.txt","r");
	char* test_block = (char*)(malloc(BLOCK_SIZE));
	int out_size = f_read(test_block,BLOCK_SIZE,1,fd);
	if(out_size > 0)
		printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
	else
		printf("something wrong about f_read or write or seek!\n");
	*/


	//test f_write a lot of things in exist file
	fd = f_open("/test.txt","a");
	char* test_text = (char*)malloc(BLOCK_SIZE * 555);
	for(int i = 0; i < 555; i ++) {
		strcat(test_text,junk_c);
	}
	printf("the length of test_text is %d\n",strlen(test_text));
	//int w_size = f_write(test_text, strlen(test_text),1,fd);
	//printf("I write %d by f_write\n",w_size);
	//get the inode region
	//  fp = fopen("test","r");
	//  if(fp == NULL){
	//  	perror("ERROR\n");
	// // 	return 0;
	//  }
	// fseek(fp, 0, SEEK_END);
 //   	size_t size = ftell(fp);
	// fseek(fp,0,SEEK_SET);
	// file_buffer = (char*) malloc(size);
	// fread(file_buffer, size ,1,fp);
 //  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
 //  	//print_free_blocks(file_buffer,sp);
	// //print_superblock(sp);
	// inode* inode_head = (inode*)(file_buffer + inode_start);
 //    inode* root = inode_head;
    //print_inode_region(sp,file_buffer);
    int w_size = f_write(test_text, strlen(test_text),1,fd);
    // fseek(fp,0,SEEK_SET);
    // fread(file_buffer, size ,1,fp);
   	//print_free_blocks(file_buffer,sp);
    //print_inode_region(sp,file_buffer);
    //go to the block to see what is in there
    // char* t_block = (char*) malloc(BLOCK_SIZE);
    // int data_address = inode_end + 10 * BLOCK_SIZE;
    // fseek(fp,data_address,SEEK_SET);
    // int test_size = fread(t_block,BLOCK_SIZE,1,fp);
    // if(test_size > 0)
    // 	printf("the content of test_block is %s\n",t_block);
	//use f_seek to go back
	//we should have pre define
	//f_seek(fd,10,"SEEK_SET");
    f_close(fd);
    fd = f_open("/test.txt","r");
	char* test_block = (char*)(malloc(BLOCK_SIZE*555));
	int out_size = f_read(test_block,BLOCK_SIZE*555,1,fd);
	int count = 0;
	for (int i = 0; i < BLOCK_SIZE*555; i++){
		if (test_block[i] != 'k') {
			count++;
		}
	}
	printf("The diff is %d\n, shit!", count);
	if(out_size > 0)
		//printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
		printf("hello\n");
	else
		printf("something wrong about f_read or write or seek!\n");

	f_close(fd);
	// //one more write
	// fd = f_open("/test.txt","a");
	// w_size = f_write(test_text, strlen(test_text),1,fd);
	// printf("the write size is %d\n",w_size);
	// fseek(fp,0,SEEK_SET);
	// fread(file_buffer, size ,1,fp);
	// print_inode_region(sp,file_buffer);	
	// print_directory(sp,root,file_buffer);
	
	//f_close(fd);
	/*
	bzero(test_block,sizeof(test_block));
	fd = f_open("/test.txt","r");
	for(int i = 0; i < 20; i ++) {
		f_seek(fd,BLOCK_SIZE*i,"SEEK_SET");
		out_size = f_read(test_block,BLOCK_SIZE,1,fd);
		if(out_size > 0)
			printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
		else
			printf("something wrong about f_read or write or seek!\n");
	}
	*/
	//print_free_blocks(file_buffer,sp);
	return 0;
}