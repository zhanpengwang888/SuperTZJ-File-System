
#include "test_util.h"
#include "../fs.h"
using namespace std;

int fd;
FILE* fp;
char* file_buffer;
Superblock *sp; // super block
//file_node *open_file_table[MAX_OPEN_FILE];
inode *disk_inode_list[MAX_INODE_NUM];

void create_node(inode* test_inode, int index, int size, string name){
	test_inode->nlink = 1;
	test_inode->permission = 7;
	test_inode->type = NORMAL_FILE;
	test_inode->next_inode = -1;
	test_inode->size = size;
	test_inode->uid = 0;
	test_inode->gid = 0;
	test_inode->parent = 0;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		if(i == 0)
			test_inode->dblocks[i] = 1;
		else
			test_inode->dblocks[i] = -1;
	}

	for (int i = 0; i < N_IBLOCKS; i++)
	{
		test_inode->iblocks[i] = -1;
	}

	test_inode->i2block = -1;
	test_inode->i3block = -1;
	//can't directly assign, use strcpy
	//default_inode->file_name = "";
	std::strcpy(test_inode->file_name, name.c_str());

}
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
	int t_size = fread(file_buffer, size, 1,fp);
  	sp = (Superblock*)(file_buffer + BOOT_SIZE);
	print_superblock(sp);
	int inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
    int inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
    int inode_num = (inode_end - inode_start) / INODE_SIZE;
    //print root directory's entries
    inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    //print_inode(root,0,1);
    //print_directory(sp,root,file_buffer);
 //    for(int i = 0; i < inode_num ; i++) {
 //       fseek(fp,inode_start + i * INODE_SIZE,SEEK_SET);
 // 	   disk_inode_list[i] = (inode*)(malloc(INODE_SIZE));
 // 	   fread(disk_inode_list[i],INODE_SIZE,1,fp);
 // 	   //print_inode(disk_inode_list[i],i,1);
	// }
    //create test file
	char* test_block = (char*) malloc(BLOCK_SIZE);
	strcpy(test_block,"Mark is the best!");
	int s_size = strlen(test_block);
	inode *test_inode = (inode *)malloc(sizeof(inode));
	char* f_name = "test.txt";
	create_node(test_inode,1,s_size,"test.txt");
	print_inode(test_inode,1,1);
	//write inode into disk
	fseek(fp,inode_start+1*INODE_SIZE,SEEK_SET);
	fwrite(test_inode,INODE_SIZE,1,fp);
	int data_address = inode_end + 1 * BLOCK_SIZE;
	fseek(fp,data_address,SEEK_SET);
	fwrite(test_block,BLOCK_SIZE,1,fp);

	//change root directory entry
	int r_address = inode_end;
	if(fseek(fp, r_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
 	}
 	char* root_block = (char*)(malloc(BLOCK_SIZE));
 	fread(root_block,BLOCK_SIZE,1,fp);
 	directory_entry* root_table = (directory_entry*)(root_block);
 	strcpy(root_table[2].file_name, f_name);
 	root_table[2].inode_entry = 1;
 	cout << root_table[2].file_name << root_table[2].inode_entry << endl;
 	cout << root_table[1].file_name << root_table[1].inode_entry << endl;
	if(fseek(fp, r_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
 	}
 	fwrite(root_block,BLOCK_SIZE,1,fp);
 	//update root inode
 	root-> size += sizeof(directory_entry);
 	fseek(fp,inode_start,SEEK_SET);
 	fwrite(root,INODE_SIZE,1,fp);
 	//refresh buffer
 	fseek(fp, 0, SEEK_SET);
 	t_size = fread(file_buffer, size, 1,fp);
 	print_directory(sp,root,file_buffer);
 	//print the test_block
 	fseek(fp,data_address,SEEK_SET);
 	fread(test_block,BLOCK_SIZE,1,fp);
 	cout << test_block << endl;
  //read from the file descriptor
	




}

int main() {
	//f_open("/usr/doc/abc.txt", "r");
	format_default_size("test");
	create_test_file("test");
	//assume mount to root directory
	f_mount("/","test");
	int test_fd = f_open("/test.txt","r");
	char* test_buffer = (char*) malloc(BLOCK_SIZE);
	int out_size = f_read(test_buffer,10,1,test_fd) >0;
	if(out_size > 0) {
		printf("test_buffer's size is %d, content %s\n",out_size,test_buffer); 
	}
	return 0;
}
