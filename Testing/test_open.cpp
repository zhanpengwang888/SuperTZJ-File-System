
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
// void update_sp() {
// 	fseek(fp,BOOT_SIZE,SEEK_SET);
// 	fwrite(sp,sizeof(Superblock),1,fp);
// }
// void create_mid_node(inode* test_inode, int index, int size, string name){
// 	test_inode->nlink = 1;
// 	test_inode->permission = 7;
// 	test_inode->type = NORMAL_FILE;
// 	test_inode->next_inode = -1;
// 	test_inode->size = size;
// 	test_inode->uid = 0;
// 	test_inode->gid = 0;
// 	test_inode->parent = 0; //parent is root
// 	for (int i = 0; i < N_DBLOCKS; i++)
// 	{
// 			test_inode->dblocks[i] = i + 1;
// 	}

// 	for (int i = 0; i < N_IBLOCKS; i++)
// 	{
// 		test_inode->iblocks[i] = -1;
// 	}

// 	test_inode->i2block = -1;
// 	test_inode->i3block = -1;
// 	//can't directly assign, use strcpy
// 	//default_inode->file_name = "";
// 	std::strcpy(test_inode->file_name, name.c_str());

// }

// void create_node(inode* test_inode, int index, int size, string name){
// 	test_inode->nlink = 1;
// 	test_inode->permission = 7;
// 	test_inode->type = NORMAL_FILE;
// 	test_inode->next_inode = -1;
// 	test_inode->size = size;
// 	test_inode->uid = 0;
// 	test_inode->gid = 0;
// 	test_inode->parent = 0;
// 	for (int i = 0; i < N_DBLOCKS; i++)
// 	{
// 		if(i == 0)
// 			test_inode->dblocks[i] = 1;
// 		else
// 			test_inode->dblocks[i] = -1;
// 	}

// 	for (int i = 0; i < N_IBLOCKS; i++)
// 	{
// 		test_inode->iblocks[i] = -1;
// 	}

// 	test_inode->i2block = -1;
// 	test_inode->i3block = -1;
// 	//can't directly assign, use strcpy
// 	//default_inode->file_name = "";
// 	std::strcpy(test_inode->file_name, name.c_str());

// }

// void create_mid_file(char* name) {
// 	fp = fopen(name,"r+");
// 	if(fp == NULL)
// 		perror("Error\n");
// 	fseek(fp, 0, SEEK_END);
//   	size_t size = ftell(fp);
//   	fseek(fp, 0, SEEK_SET);
//   	file_buffer = (char*)malloc(sizeof(char) * size + 1);
// 	int t_size = fread(file_buffer, size, 1,fp);
//   	sp = (Superblock*)(file_buffer + BOOT_SIZE);
// 	inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
//     inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
//     inode_num = (inode_end - inode_start) / INODE_SIZE;
//     //print root directory's entries
//     inode* inode_head = (inode*)(file_buffer + inode_start);
//     inode* root = inode_head;

//     //create the test file and inode -- not write in yet 
//     char* test_block = (char*) malloc(BLOCK_SIZE);
// 	strcpy(test_block,junk_c);
// 	int s_size = strlen(test_block);
// 	inode *test_inode = (inode *)malloc(sizeof(inode));
// 	char* f_name = "test.txt";
// 	create_mid_node(test_inode,1,s_size*10,"test.txt");
// 	//write test inode into disk
// 	fseek(fp,inode_start+1*INODE_SIZE,SEEK_SET);
// 	fwrite(test_inode,INODE_SIZE,1,fp);
// 	//put the test blocks list into file
// 	int data_address;
// 	for(int i = 1; i < 11; i++) {
// 		data_address = inode_end + i * BLOCK_SIZE;
// 		fseek(fp,data_address,SEEK_SET);
// 		fwrite(test_block,BLOCK_SIZE,1,fp);
// 	}

// 	//change root directory entry
// 	int root_address = inode_end;
// 	if(fseek(fp, root_address,SEEK_SET) < 0) {
//       perror("lseek fails\n");
//       return;
//  	}
//  	char* root_block = (char*)(malloc(BLOCK_SIZE));
//  	fread(root_block,BLOCK_SIZE,1,fp);
//  	directory_entry* root_table = (directory_entry*)(root_block);
//  	strcpy(root_table[2].file_name, f_name);
//  	root_table[2].inode_entry = 1;
//  	//cout << root_table[2].file_name << root_table[2].inode_entry << endl;
//  	//cout << root_table[1].file_name << root_table[1].inode_entry << endl;
// 	if(fseek(fp, root_address,SEEK_SET) < 0) {
//       perror("lseek fails\n");
//       return;
//  	}
//  	fwrite(root_block,BLOCK_SIZE,1,fp);
//  	//update root inode
//  	root-> size += sizeof(directory_entry);
//  	fseek(fp,inode_start,SEEK_SET);
//  	fwrite(root,INODE_SIZE,1,fp);


//  	//update superblock
//  	sp->free_inode = 2;
//  	sp->free_block = 11;
//  	//print_superblock(sp);
//  	update_sp();
//  	print_superblock(sp);
//  	fclose(fp);
// }
// //get a disk image and modify to be test disk image
// void create_test_file(char* name) {
// 	fp = fopen(name,"r+");
// 	if(fp == NULL)
// 		perror("Error\n");
// 	fseek(fp, 0, SEEK_END);
//   	size_t size = ftell(fp);
//   	fseek(fp, 0, SEEK_SET);
//   	cout << "the size of file is " << size <<endl;
//   	file_buffer = (char*)malloc(sizeof(char) * size + 1);
// 	int t_size = fread(file_buffer, size, 1,fp);
//   	sp = (Superblock*)(file_buffer + BOOT_SIZE);
// 	print_superblock(sp);
// 	inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
//     inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
//     inode_num = (inode_end - inode_start) / INODE_SIZE;
//     //print root directory's entries
//     inode* inode_head = (inode*)(file_buffer + inode_start);
//     inode* root = inode_head;
//     //print_inode(root,0,1);
//     //print_directory(sp,root,file_buffer);
//  //    for(int i = 0; i < inode_num ; i++) {
//  //       fseek(fp,inode_start + i * INODE_SIZE,SEEK_SET);
//  // 	   disk_inode_list[i] = (inode*)(malloc(INODE_SIZE));
//  // 	   fread(disk_inode_list[i],INODE_SIZE,1,fp);
//  // 	   //print_inode(disk_inode_list[i],i,1);
// 	// }
//     //create test file
// 	char* test_block = (char*) malloc(BLOCK_SIZE);
// 	strcpy(test_block,"Mark is the best!");
// 	int s_size = strlen(test_block);
// 	inode *test_inode = (inode *)malloc(sizeof(inode));
// 	char* f_name = "test.txt";
// 	create_node(test_inode,1,s_size,"test.txt");
// 	//print_inode(test_inode,1,1);
// 	//write inode into disk
// 	fseek(fp,inode_start+1*INODE_SIZE,SEEK_SET);
// 	fwrite(test_inode,INODE_SIZE,1,fp);
// 	int data_address = inode_end + 1 * BLOCK_SIZE;
// 	fseek(fp,data_address,SEEK_SET);
// 	fwrite(test_block,BLOCK_SIZE,1,fp);

// 	//change root directory entry
// 	int r_address = inode_end;
// 	if(fseek(fp, r_address,SEEK_SET) < 0) {
//       perror("lseek fails\n");
//       return;
//  	}
//  	char* root_block = (char*)(malloc(BLOCK_SIZE));
//  	fread(root_block,BLOCK_SIZE,1,fp);
//  	directory_entry* root_table = (directory_entry*)(root_block);
//  	strcpy(root_table[2].file_name, f_name);
//  	root_table[2].inode_entry = 1;
//  	//cout << root_table[2].file_name << root_table[2].inode_entry << endl;
//  	//cout << root_table[1].file_name << root_table[1].inode_entry << endl;
// 	if(fseek(fp, r_address,SEEK_SET) < 0) {
//       perror("lseek fails\n");
//       return;
//  	}
//  	fwrite(root_block,BLOCK_SIZE,1,fp);
//  	//update root inode
//  	root-> size += sizeof(directory_entry);
//  	fseek(fp,inode_start,SEEK_SET);
//  	fwrite(root,INODE_SIZE,1,fp);
//  	//refresh buffer -- for testing
//  	fseek(fp, 0, SEEK_SET);
//  	t_size = fread(file_buffer, size, 1,fp);
//  	print_directory(sp,root,file_buffer);

//  	//print the test_block
//  	// fseek(fp,data_address,SEEK_SET);
//  	// fread(test_block,BLOCK_SIZE,1,fp);
//  	// cout << test_block << endl;
//   //read from the file descriptor
	
//  	//update the superblock into the disk
//  	sp->free_inode = 2;
//  	sp->free_block = 2;
//  	//print_superblock(sp);
//  	update_sp();

//  	//test update_sp();
//  	/*
//  	fclose(fp);
//  	fp = fopen("test","r+");
//  	fseek(fp, 0, SEEK_SET);
//  	t_size = fread(file_buffer, size, 1,fp);
//  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
//  	print_superblock(sp);
//  	*/
//  	fclose(fp);


// }

int main() {
	//f_open("/usr/doc/abc.txt", "r");
	format_default_size("test");

	//for testing small file
	create_test_file("test");

	//for testing mid size file
	//create_mid_file("test");
	//assume mount to root directory
	f_mount("/","test");
	/*
	//test F_open read mode
	int test_fd = f_open("/test.txt","r");
	char* test_buffer = (char*) malloc(BLOCK_SIZE);
	int out_size = f_read(test_buffer,500,1,test_fd);
	if(out_size > 0) {
		printf("test_buffer's size is %d, content %s\n",out_size,test_buffer); 
	}
	f_close(test_fd);
	out_size = f_read(test_buffer,500,1,test_fd);
	printf("out size is %d\n",out_size);
	*/

	
	//test f_remove small file
	/*
	int test_fd = f_open("/test.txt","r");
	f_close(test_fd);
	f_remove("/test.txt");
	printf("Testing program printing !!! $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	char* test_block = (char*) malloc(BLOCK_SIZE);
	fp = fopen("test","r");
	int data_address = inode_end + 1 * BLOCK_SIZE;
	fseek(fp,data_address,SEEK_SET);
	fread(test_block,BLOCK_SIZE,1,fp);
	int* free_one = (int*) test_block;
	printf("the next free block is %d\n",free_one[0]);
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
	*/
	
	
	/*
	//test write mode with small file
	int test_fd = f_open("/test2.txt","w");
	//check root directory
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
    print_directory(sp,root,file_buffer);
    char* test_text = "It is the choice of Steins Gate!\n";
	//test f_write here
	int w_size = f_write(test_text, strlen(test_text),1,fd);
	printf("I write %d by f_write\n",w_size);
	fread(file_buffer, size ,1,fp);
	//print_inode_region(sp,file_buffer);
	//test content in it
	f_close(fd);
    fd = f_open("/test2.txt","r");
	char* test_block = (char*)(malloc(BLOCK_SIZE));
	int out_size = f_read(test_block,BLOCK_SIZE,1,fd);
	if(out_size > 0)
		printf("the content we get from the test block is %s, its size is %d\n",test_block,out_size);
	else
		printf("something wrong about f_read or write or seek!\n");
	*/
	/*
	//test F_open and read with middle file
	int test_fd = f_open("/test.txt","r");
	char* test_buffer = (char*) malloc(BLOCK_SIZE);
	int out_size = f_read(test_buffer,6000,1,test_fd);
	if(out_size > 0) {
		printf("test_buffer's size is %d, content %s\n",out_size,test_buffer); 
	}
	f_close(test_fd);
	out_size = f_read(test_buffer,500,1,test_fd);
	printf("out size is %d\n",out_size);
	*/

	//test f_remove middle size file
	/*
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
    */


	return 0;
}
