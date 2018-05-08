#include "fs.h"
#include "Testing/test_util.h"

Superblock *sb; // super block
file_node *open_file_table[MAX_OPEN_FILE];
inode *disk_inode_region[MAX_INODE_NUM];
int cur_directory;
int num_of_total_data_block;
int num_of_total_inode;
int num_of_open_file;
int disk;
int open_directory_tracker[MAX_OPEN_FILE];

using namespace std;

//temporary prototypes -- need to combine in one util.c file
int create_file(const string filename, int parent_inode, int type, int mode);
void clean_inode(inode* cur, int index);
void update_sb();
void clean_block(int index);
int get_index(int indirect_idx, int offset);
int get_index_by_offset(inode* f_node, int offset);
void clean_file(inode* f_node);
vector<string> split(const string &s, char delim);
int add_to_file_table(int inode_num, inode *f_node,int mode);
int traverse_dir(int dirinode_index, string filename, bool isLast);
int get_next_free_block(int block_index);
int remove_p_dir_entry(char* file_name,int parent_index);
int swap_entry(int dirinode_index, string filename);
directory_entry* deal_last_entry(int dirinode_index,const char* filename);

//testing function
void print_file_table();
void print_file_status(int fd);

// update root in superblock, update inode, open root directory

int f_mount(char* destination, char* diskname) {
	int fd;
	fd = open(diskname, O_RDWR);
	disk = fd;
	if (fd == -1) {
		fd = format_default_size(diskname);
		if (fd == -1) {
			return EXIT_FAILURE;
		}
		else {
			open(diskname, O_RDWR);
		}
	}
	disk = fd; //set disk to fd
	//zero will always be root directory
	//read in superblock here
	//if superblock has not been initialized
	if(sb == NULL) {
		sb = (Superblock *)malloc(sizeof(Superblock));
	}
	//read the stuff into superblock
	lseek(disk,BOOT_SIZE,SEEK_SET);
	read(disk,sb,sizeof(Superblock));
	sb->root = 0;
	//printf("f_mount testing print************************************\n");
	//get file size of disk
	off_t filesize = 0;
	filesize = lseek(fd, 0, SEEK_END);
	//filesize = ftell(fd);
	lseek(fd, 0, SEEK_SET);
	//printf("the total size of this file is %lld\n",filesize);
	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);
	
	for (int i = 0; i < num_of_total_inode; ++i) {
		disk_inode_region[i] = (inode*)malloc(sizeof(inode));
		lseek(fd, BOOT_SIZE + SUPER_SIZE + i * sizeof(inode), SEEK_SET);
		read(fd, disk_inode_region[i], sizeof(inode));
	}
	//update root directory
	disk_inode_region[0]->nlink = 1;
	std::strncpy(disk_inode_region[0]->file_name, destination, sizeof(disk_inode_region[0]->file_name));
	lseek(fd, BOOT_SIZE + SUPER_SIZE, SEEK_SET);
	write(fd, disk_inode_region[0], sizeof(inode));
	//set up open file table
	for (int i = 0; i < MAX_OPEN_FILE; i++) {
		open_file_table[i] = (file_entry*)malloc(sizeof(file_node));
		open_file_table[i]->inode_entry = -1;
		open_file_table[i]->block_offset = 0;
		open_file_table[i]->byte_offset = 0;
	}
}


// free everything that has been malloced
int f_unmount(char* diskname) {
	for (int i = 0; i < MAX_OPEN_FILE; i++) {
		free(open_file_table[i]);
	}
	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);
	for (int i = 0; i < num_of_total_inode; ++i) {
		free(disk_inode_region[i]);
	}
	free(sb);
	close(disk);
	return SUCCESS;
}


void create_root_dir(inode* rt_node) {
  printf("I am here\n");
  size_t data_address = BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE;
  //create buffer to index_table of indirect block
  char* dir_buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
  //lseek to specific position of the disk image and read from it -- make sure disk is valid
  if(lseek(disk, data_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
  }
  //read from the file descriptor
  read(disk,dir_buffer,BLOCK_SIZE);
  directory_entry* entry_table = (directory_entry*)(dir_buffer); //here we need to read from disk image
  entry_table[0].inode_entry = 0;
  strcpy(entry_table[0].file_name,".");
  entry_table[1].inode_entry = 0;
  strcpy(entry_table[1].file_name,"..");
  //printf("what happen?\n");
  //printf("the entry 0 has %s\n",entry_table[0].file_name);
  //printf("the entry 1 has %s\n",entry_table[1].file_name);
  lseek(disk, data_address,SEEK_SET);
  int s_byte = write(disk,dir_buffer,BLOCK_SIZE);
  // if(s_byte <= 0) {
  // 	perror("Error\n");
  // }
  // else
  // 	printf("%d\n",s_byte);
  free(dir_buffer);
}

int format_default_size(string filename)
{
	int fd;
	//in open we need to convert to c-style string
	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT,0777);
	if (fd == -1)
	{
		return EXIT_FAILURE;
	}
	disk = fd;
	ftruncate(fd, DEFAULT_SIZE);
	char *bootblock = (char *)malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	int out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE)
	{
		return EXIT_FAILURE;
	}
	std::free(bootblock);
	sb = (Superblock *)malloc(sizeof(Superblock));
	sb->size = BLOCK_SIZE;
	sb->inode_offset = 0;
	sb->data_offset = DEFAULT_SIZE * INODE_RATE / BLOCK_SIZE;
	sb->free_inode = 1;
	sb->free_block = 1;

	num_of_total_data_block = (DEFAULT_SIZE - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
	//instead of fseek, we should use lseek for file descriptor
	lseek(fd, BOOT_SIZE, SEEK_SET);
	write(fd, sb, SUPER_SIZE);

	for (int i = 0; i < num_of_total_data_block; i++)
	{
		if (i == num_of_total_data_block - 1)
		{
			int to_write = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &to_write, sizeof(int));
		}
		else
		{
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &i, sizeof(int));
		}
	}

	// construct a default inode
	inode *default_inode = (inode *)malloc(sizeof(inode));
	default_inode->nlink = 0;
	default_inode->permission = 7;
	default_inode->type = NORMAL_FILE;
	default_inode->next_inode = 1;
	default_inode->size = 0;
	default_inode->uid = 0;
	default_inode->gid = 0;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		default_inode->dblocks[i] = -1;
	}

	for (int i = 0; i < N_IBLOCKS; i++)
	{
		default_inode->iblocks[i] = -1;
	}

	default_inode->i2block = -1;
	default_inode->i3block = -1;
	//can't directly assign, use strcpy
	//default_inode->file_name = "";
	std::strcpy(default_inode->file_name, "");
	//default_inode->padding = 0;

	//root directory inode
	inode* root_inode = (inode *)malloc(sizeof(inode));
	root_inode->nlink = 0;
	root_inode->permission = RDONLY;
	root_inode->type = DIRECTORY_FILE;
	root_inode->next_inode = -1;
	root_inode->size = sizeof(directory_entry) * 2; // for . and ..
	root_inode->uid = 0;
	root_inode->gid = 0;

	for (int i = 0; i < N_DBLOCKS; i++)
	{
		if(i == 0)
			root_inode->dblocks[i] = 0;
		else
		  root_inode->dblocks[i] = -1;
	}

	for (int i = 0; i < N_IBLOCKS; i++)
	{
		root_inode->iblocks[i] = -1;
	}

	root_inode->i2block = -1;
	root_inode->i3block = -1;
	//root directory name is /
	std::strcpy(root_inode->file_name, "/");
	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i)
	{
		//the first one will be root directory
		if(i == 0) {
			root_inode->next_inode = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, root_inode, sizeof(inode));
			continue;
		}

		if (i == num_of_total_inode - 1)
		{
			default_inode->next_inode = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
		else
		{
			default_inode->next_inode = i + 1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
	}
	create_root_dir(root_inode);
	//test whether root_dir has been successfully write in
	char* test_buffer = (char*) malloc(BLOCK_SIZE);
	lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset*BLOCK_SIZE, SEEK_SET);
	read(fd,test_buffer,BLOCK_SIZE);
	directory_entry* entry_table = (directory_entry*)test_buffer;
	//cout << "from file:" << entry_table[0].file_name << entry_table[1].inode_entry << endl;
	//cout << "from file:" << entry_table[1].file_name << entry_table[1].inode_entry << endl;
	std::free(default_inode);
	free(root_inode);
	close(fd);
	//need to replace by predefined
	return 0;
}

int format_with_given_size(string filename, long int file_size)
{
	int fd;
	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT,0777);
	if (fd == -1)
	{
		return EXIT_FAILURE;
	}
	disk = fd;
	ftruncate(fd, file_size);
	char *bootblock = (char *)malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	int out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE)
	{
		return EXIT_FAILURE;
	}
	std::free(bootblock);
	sb = (Superblock *)malloc(sizeof(Superblock));
	sb->size = BLOCK_SIZE;
	sb->inode_offset = 0;
	sb->data_offset = file_size * INODE_RATE / BLOCK_SIZE;
	sb->free_inode = 0;
	sb->free_block = 0;

	num_of_total_data_block = (file_size - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
	lseek(fd, BOOT_SIZE, SEEK_SET);
	write(fd, sb, SUPER_SIZE);

	for (int i = 0; i < num_of_total_data_block; i++)
	{
		if (i == num_of_total_data_block - 1)
		{
			int to_write = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &to_write, sizeof(int));
		}
		else
		{
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &i, sizeof(int));
		}
	}

	// construct a default inode
	inode *default_inode = (inode *)malloc(sizeof(inode));
	default_inode->nlink = 0;
	default_inode->permission = 7;
	default_inode->type = NORMAL_FILE;
	default_inode->next_inode = 1;
	default_inode->size = 0;
	default_inode->uid = 0;
	default_inode->gid = 0;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		default_inode->dblocks[i] = 0;
	}

	for (int i = 0; i < N_IBLOCKS; i++)
	{
		default_inode->iblocks[i] = 0;
	}

	default_inode->i2block = 0;
	default_inode->i3block = 0;
	//default_inode->file_name = "";
	std::strcpy(default_inode->file_name, "");
	//default_inode->padding = 0;

	//root directory inode
	inode* root_inode = (inode *)malloc(sizeof(inode));
	root_inode->nlink = 0;
	root_inode->permission = RDONLY;
	root_inode->type = DIRECTORY_FILE;
	root_inode->next_inode = -1;
	root_inode->size = sizeof(directory_entry) * 2; // for . and ..
	root_inode->uid = 0;
	root_inode->gid = 0;

	for (int i = 0; i < N_DBLOCKS; i++)
	{
		if(i == 0)
			root_inode->dblocks[i] = 0;
		else
		  root_inode->dblocks[i] = -1;
	}

	for (int i = 0; i < N_IBLOCKS; i++)
	{
		root_inode->iblocks[i] = -1;
	}

	root_inode->i2block = -1;
	root_inode->i3block = -1;
	//root directory name is /
	std::strcpy(root_inode->file_name, "/");

	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i)
	{
				//the first one will be root directory
		if(i == 0) {
			root_inode->next_inode = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, root_inode, sizeof(inode));
			continue;
		}

		if (i == num_of_total_inode - 1)
		{
			default_inode->next_inode = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
		else
		{
			default_inode->next_inode = i + 1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
	}
	create_root_dir(root_inode);
	std::free(default_inode);
	free(root_inode);
	close(fd);
	//need to replace by predefined
	return 0;
}

// we need to parse the command to see which format we will use


int f_seek(int fd, long int offset, char* whence) {
	printf("here is f_seek test printing!! &&&&&&&&&&&&&&&&&&&&&&\n");
	printf("the whence is whence is %s\n",whence);
	print_file_status(fd);
	if (fd < 0 or fd > MAX_OPEN_FILE) {
		return EXIT_FAILURE;
	}
	file_node* cur_file = open_file_table[fd];
	if (cur_file == NULL) {
		// the file has not been opened
		return EXIT_FAILURE;
	}
	int inode_idx = cur_file->inode_entry;
	print_inode(disk_inode_region[inode_idx],inode_idx,1);
	if (inode_idx > num_of_total_inode) {
		// the file doesn't exist
		return EXIT_FAILURE;
	}
	else if (disk_inode_region[inode_idx]->nlink < 1) {
		return EXIT_FAILURE;
	}
	else {
		if (strcmp(whence, "SEEK_SET") == 0) {
			printf("it comes in SEEK_SET\n");
			if (offset > disk_inode_region[inode_idx]->size) {
				return EXIT_FAILURE;
			}
			cur_file->block_offset = offset / BLOCK_SIZE + 1;
			cur_file->byte_offset = offset % BLOCK_SIZE;
			cur_file->block_index = get_index_by_offset(disk_inode_region[inode_idx], cur_file->block_offset);
			printf("if finishes its job in SEEK_SET\n");
		}
		else if (strcmp(whence, "SEEK_CUR") == 0) {
			long int to_modify = cur_file->block_offset * BLOCK_SIZE + cur_file->byte_offset + offset;
			if (to_modify > disk_inode_region[inode_idx]->size) {
				return EXIT_FAILURE;
			}
			cur_file->block_offset = to_modify / BLOCK_SIZE + 1;
			cur_file->byte_offset = to_modify % BLOCK_SIZE;
			cur_file->block_index = get_index_by_offset(disk_inode_region[inode_idx], cur_file->block_offset);
		}
		else if (strcmp(whence, "SEEK_END") == 0) {
			if (offset > 0) {
				return EXIT_FAILURE;
			}
			long int to_modify = disk_inode_region[inode_idx]->size + offset;
			cur_file->block_offset = to_modify / BLOCK_SIZE + 1;
			cur_file->byte_offset = to_modify % BLOCK_SIZE;
			cur_file->block_index = get_index_by_offset(disk_inode_region[inode_idx], cur_file->block_offset);
		}
	}
	printf("the current fd after f_seek is: \n");
	print_file_status(fd);
	return EXIT_SUCCESS;
}


void rewind(int fd) {
	f_seek(fd, 0, "SEEK_SET");
	// return EXIT_SUCCESS;
}

void clean_inode(inode* cur, int index) {
	cur->nlink = 0;
    cur->next_inode = sb->free_inode; //the next free inode is the head of free inode list
    cur->permission = 0;
    cur->type = -1;
    cur->parent = -1;
    cur->size = 0;
    cur->uid = 0;
    cur->gid = 0;
    for(int i = 0; i < N_DBLOCKS; i++) {
      cur->dblocks[i] = 0;
    }
    for(int i = 0; i < N_IBLOCKS; i++) {
      cur->iblocks[i] = 0;
    }
    cur->i2block = 0;
    cur->i3block = 0;
    strcpy(cur->file_name,""); //clear the file name
    //update the free inode list in superblock
    sb->free_inode = index;
}

//update the superblock and write in disk image
void update_sb() {
	lseek(disk,BOOT_SIZE,SEEK_SET);
	write(disk,sb,sizeof(Superblock));
}
//get the index of block, read corresponding data block in disk image and free them,
void clean_block(int index) {
	printf("clean_block test printing !!! &&&&&&&&&&&&&&&&&");
	printf("the current head of free_block list is %d\n",sb->free_block);
	size_t data_start = BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE;
  	size_t data_address = data_start + index*BLOCK_SIZE;
  	//get the data block
  	char* data_buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
  	if(lseek(disk, data_address,SEEK_SET) < 0) {
  		cout << "There is something wrong in lseek of clean block" << endl;
  		return;
  	}
  	read(disk,data_buffer,BLOCK_SIZE);
  	//clean the data in it
  	bzero(data_buffer, BLOCK_SIZE);
  	int* free_block = (int*) data_buffer;
  	//write back to disk
  	free_block[0] = sb->free_block;
  	if(lseek(disk, data_address,SEEK_SET) < 0){
  		cout << "There is something wrong in lseek of clean block" << endl;
		return;
	}
	write(disk,free_block,BLOCK_SIZE);
  	//set the next free block to be the current head of free block list in superblock
  	//update superblock's free block element
  	sb->free_block = index;
  	printf("after free, the current head of free_block list is %d\n",sb->free_block);
  	//do we need to update superblock in the disk? ----- NOT DONE YET !!!!!!!!
  	free(data_buffer);
}

//get the specific block index from the indirect block
int get_index(int indirect_idx, int offset) {
  //data start might be stored somewhere if necessary
  size_t data_start = BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE;
  size_t data_address = data_start + indirect_idx*BLOCK_SIZE;
  //create buffer to index_table of indirect block
  char* index_buffer = (char*) malloc(sizeof(char) * BLOCK_SIZE);
  //lseek to specific position of the disk image and read from it -- make sure disk is valid
  if(lseek(disk, data_address,SEEK_SET) < 0)
  	return FAIL;
  //read from the file descriptor
  read(disk,index_buffer,BLOCK_SIZE);
  int* index_table = (int*)(index_buffer); //here we need to read from disk image
  free(index_buffer);
  return index_table[offset];
}

//get the specific block index by the inode pointer and the block offset in the file, offset should be positive
//if succeed, return the data block index
//if fail, return -1
//update Sun 4pm
int get_index_by_offset(inode* f_node, int offset) {
	int entry_num = BLOCK_SIZE/sizeof(int);
	int d_limit = N_DBLOCKS;
	int i_limit = d_limit + N_IBLOCKS * entry_num;
	int i2_limit = i_limit + entry_num * entry_num;
	int i3_limit = i2_limit + entry_num * entry_num * entry_num;
	int index = offset - 1;
	int b_index, f_index, s_index;
	int f_offset, s_offset;
	if(offset <= 0) {
		cout << "No nonpositive offset!" << endl;
		return FAIL;
	}
	else if(offset > 0 && offset <= d_limit) {
		return f_node->dblocks[index];
	}
	else if(offset > d_limit && offset <= i_limit) {
		int i_elem = index - d_limit; //know the exact position in indirect block list
		b_index = get_index(f_node->iblocks[i_elem/entry_num],i_elem%entry_num);
		return b_index;
	}
	else if(offset > i_limit && offset <= i2_limit) {
		int i2_elem = index - i_limit; //only consider in the i2 block's entry
		//get the first level indirect block index stroed in second level
		f_index = get_index(f_node->i2block,i2_elem/entry_num);
		f_offset = i2_elem%entry_num;
		return get_index(f_index, f_offset);
	}
	else if(offset > i2_limit && offset < i3_limit) {
		int i3_elem = index - i2_limit;
		s_index = get_index(f_node->i3block,i3_elem/(entry_num*entry_num)); //find the second indirect block index in third level indirect block
		s_offset = i3_elem%(entry_num*entry_num);
		f_index = get_index(s_index,s_offset/entry_num);
		f_offset = s_offset%entry_num;
		return get_index(f_index,f_offset);
	}
	else {
		cout << "Invalid offset number, please check it again" <<endl;
		return FAIL;
	}
}

//dir functions
void clean_file(inode* f_node) {
	//first we need to go to each data block and link to the free block list
	//consecutively update the free block in superblock
	//we need to set up a system that keep track of 
	int remain = f_node->size;
  	int num_indir_idx = BLOCK_SIZE / sizeof(int);
  //arrange the direct blocks
  printf("******************Cleaning direct blocks*******************\n");
  for(int i = 0; i < N_DBLOCKS ; i++) {
    clean_block(f_node->dblocks[i]);
    remain = remain - BLOCK_SIZE;
    if(remain <= 0)
      return;
  }

  //arrange indirect blocks
  //arrange first level indirect blocks
  printf("*********************clearing indirect blocks************************\n");
  for(int i = 0; i < N_IBLOCKS; i++) {
    // int index_arr[num_indir_idx];
    // bzero(index_arr,sizeof(index_arr));
    for(int j = 0; j < num_indir_idx ;j++) {
      //get the index of the data block we want
      int b_index = get_index(f_node->iblocks[i],j);
      if(b_index <= 0) {
		printf("Abnormal event, get access to invalid index block!\n");
		clean_block(f_node->iblocks[i]);
		return;
      }
      else {
		clean_block(b_index);
		remain = remain - BLOCK_SIZE;
		if(remain <= 0) {
			//we need to clean the current idirect block when we leave
			clean_block(f_node->iblocks[i]);
			return;
		}
      }
    }
    //also we need to clean the indirect block itself.
    clean_block(f_node->iblocks[i]);
  }

  //clean second level indirect block
  for(int i = 0; i < num_indir_idx; i++) {
    //get the block index of first level indirect block
    int f_index = get_index(f_node->i2block,i);
    if(f_index <0) {
      printf("Abnormal event, get access to invalid first level indirect block!\n");
      clean_block(f_node->i2block);
      return;
    }
    //array to store data block
    for(int j = 0; j < num_indir_idx ;j++) {
      //get the index of the data block we want
      int b_index = get_index(f_index,j);
      if(b_index <0) {
        printf("Abnormal event, get access to invalid index block!\n");
        clean_block(f_node->i2block);
        clean_block(f_index);
		return;
	  }
      else {
        clean_block(b_index);
        remain = remain - BLOCK_SIZE;
        if(remain <= 0) {
	      	clean_block(f_node->i2block);
	        clean_block(f_index);
	        return;
        }
      }
    }
    clean_block(f_index);
  }
  clean_block(f_node->i2block);

  //arrange triple level indirect block
  for(int k = 0; k < num_indir_idx; k++) {
    int s_index = get_index(f_node->i3block,k);
    if(s_index < 0) {
      printf("Abnormal event, get access to invalid second level indirect block!\n");
      clean_block(f_node->i3block);
      return;
    }
    for(int i = 0; i < num_indir_idx; i++) {
      //get the block index of first level indirect block
      int f_index = get_index(s_index,i);
      if(f_index < 0 ) {
		printf("Abnormal event, get access to invalid first level indirect block!\n");
		clean_block(s_index); //second level indirect block
		clean_block(f_node->i3block);
		return;
	      }
      //array to store data block
      int index_arr[num_indir_idx];
      bzero(index_arr,sizeof(index_arr));
      for(int j = 0; j < num_indir_idx ;j++) {
		//get the index of the data block we want
		int b_index = get_index(f_index,j);
		if(b_index < 0) {
		  printf("Abnormal event, get access to invalid index block!\n");
		  clean_block(f_index);
		  clean_block(s_index);
		  clean_block(f_node->i3block);
		  return;
		}
		else {
		  clean_block(b_index);
		  remain = remain - BLOCK_SIZE;
		  if(remain <= 0) {
			clean_block(f_index);
			clean_block(s_index);
			clean_block(f_node->i3block);
		  	return;
		  }
		}
      }
      clean_block(f_index);
    }
    clean_block(s_index);
  }
  clean_block(f_node->i3block);
}

int traverse_directory(int dirinode_index, string directory_name) {
// first, check if the given inode has an entry in the disk inode region (i.e., Check if the directory exists or not)
	inode *direct = disk_inode_region[dirinode_index]; // get the directory
	if (direct->nlink == 0)
	{
		printf("This directory does not exist.\n");
		return FAIL;
	}
	// then, check if the given inode is a directory inode or not
	if (direct->type != DIRECTORY_FILE)
	{
		printf("This is not a directory.\n");
		return FAIL;
	}
	// now lets check the data region to check if the given directory exists or not
	// first, look into the direct data blocks
	size_t remaining_size = direct->size; // get the size of the directory, and says it is the remaining size.
	int data_offset = sb->data_offset;	// get the data offset from the super block
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		int data_block_index = direct->dblocks[i];
		// one data block can have 16 directory entries
		size_t directories_starting_region = data_region_starting_addr + BLOCK_SIZE * data_block_index;
		for (int j = 0; j < BLOCK_SIZE / sizeof(directory_entry); j++)
		{
			// may not work! Logic seems to be fine.
			lseek(disk, directories_starting_region + j * sizeof(directory_entry), SEEK_SET);
			//char *entry_char = (char *)malloc(sizeof(directory_entry));
			char entry_char[sizeof(directory_entry)];
			read(disk, entry_char, sizeof(directory_entry));
			directory_entry *entry = (directory_entry *)entry_char;
			//directory_entry *entry = (directory_entry *)disk_buffer + directories_starting_region + j * sizeof(directory_entry); // get each directory entry
			if (string(entry->file_name) == directory_name)
			{
				return entry->inode_entry;
			}
			remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
			if (remaining_size <= 0)
			{
				return FAIL;
			}
		}
	}
	// if the remaining_size is not 0, we still need to look into other data blocks.
	// Then, look into the single indirect data blocks
	size_t ptr_addr;
	for (int i = 0; i < N_IBLOCKS; i++)
	{
		ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct->iblocks[i];
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				if (string(entry->file_name) == directory_name)
				{
					return entry->inode_entry;
				}
				remaining_size -= sizeof(directory_entry);
				if (remaining_size <= 0)
				{
					return FAIL;
				}
			}
		}
	}
	// Next, look into the double indirect data blocks
	size_t i2block_ptr_addr = data_region_starting_addr + direct->i2block * BLOCK_SIZE;
	char indirect_data_blocks_chars[BLOCK_SIZE];
	bzero(indirect_data_blocks_chars, BLOCK_SIZE);
	lseek(disk, i2block_ptr_addr, SEEK_SET);
	read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
	//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
	int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
	// loop through 128 indirect data blocks
	for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
	{
		size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, idblock_ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				if (string(entry->file_name) == directory_name)
				{
					return entry->inode_entry;
				}
				remaining_size -= sizeof(directory_entry);
				if (remaining_size <= 0)
				{
					return FAIL;
				}
			}
		}
	}

	// Finally, look into the triple indirect data blocks
	size_t i3block_ptr_addr = data_region_starting_addr + direct->i3block * BLOCK_SIZE;
	char i2blocks_char[BLOCK_SIZE];
	bzero(i2blocks_char, BLOCK_SIZE);
	lseek(disk, i3block_ptr_addr, BLOCK_SIZE);
	read(disk, i2blocks_char, BLOCK_SIZE);
	//memcpy(i2blocks_char, disk_buffer + i3block_ptr_addr, BLOCK_SIZE);
	int *i2blocks = (int *)i2blocks_char;
	// first, loop through 128 doubly indirect data blocks
	for (int o = 0; o < BLOCK_SIZE / sizeof(int); o++)
	{
		size_t i2block_ptr_addr = data_region_starting_addr + i2blocks[o] * BLOCK_SIZE;
		char indirect_data_blocks_chars[BLOCK_SIZE];
		bzero(indirect_data_blocks_chars, BLOCK_SIZE);
		lseek(disk, i2block_ptr_addr, BLOCK_SIZE);
		read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
		//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
		int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
		// loop through 128 indirect data blocks
		for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
		{
			size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
			char data_blocks_char[BLOCK_SIZE];
			bzero(data_blocks_char, BLOCK_SIZE);
			lseek(disk, idblock_ptr_addr, BLOCK_SIZE);
			read(disk, data_blocks_char, BLOCK_SIZE);
			//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
			int *direct_data_block = (int *)data_blocks_char;
			// loop through 128 direct data blocks pointers.
			for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
			{
				size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
				// loop through 16 directory entries.
				for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
				{
					lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
					char entry_char[sizeof(directory_entry)];
					read(disk, entry_char, sizeof(directory_entry));
					directory_entry *entry = (directory_entry *)entry_char;
					//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
					if (string(entry->file_name) == directory_name)
					{
						return entry->inode_entry;
					}
					remaining_size -= sizeof(directory_entry);
					if (remaining_size <= 0)
					{
						return FAIL;
					}
				}
			}
		}
	}
	// if it still doesn't return, return a -1.
	return FAIL;
}

// f_opendir
// Not tested yet. Potential buggy.
int f_opendir(string path)
{
	//we can directly use f_open to do this
	//parse the path to know the filename
	vector<string> path_list = split(path, '/');
	for (int i = 0; i < path_list.size(); i++)
	{
		cout << "This element is " << path_list[i] << endl;
	}
	int dir_node = sb->root; // starting from the root
	unsigned int counter = 0;
	for (counter = 0; counter < path_list.size(); counter++)
	{
		// traverse all the directory to see if the path is valid, i.e. if the directory exists;
		dir_node = traverse_directory(dir_node, path_list[counter]);
		if (dir_node == FAIL) {
			cout << "The file path is invalid" << endl;
			return FAIL;
		}
	}
	// check if the directory is already opened.
	for (int j = 0; j < MAX_OPEN_FILE; j++) {
		if (open_file_table[j]->inode_entry == dir_node) {
			return j;
		}
	}
	// now let's add it into the open file table
	inode *target_directory = disk_inode_region[dir_node];
	int dirfd = add_to_file_table(dir_node, target_directory,RDONLY); // add it into the open file table
	return dirfd;
}

// f_readdir explained: 
// First call on f_readdir: gives the user the current directory, "."
// Second call: gives the user the parent directory of the current directory, ".."
// Third call: give the first entry under the current directory. Fourth call gives the second entry, etc.
// Not tested yet. Potential buggy !!!!!!!!!!!!
directory_entry *f_readdir(int dirfd) {
	// check if the directory pointed by the dirfd has been openned or not
	file_node *target_directory = open_file_table[dirfd]; // get the target directory
	// Next, check if the user has the right permission to read the file.
	if (dirfd < 0 || dirfd > MAX_OPEN_FILE) {
		printf("Invalid file descriptor.\n");
		return NULL;
	} else if (target_directory->inode_entry == -1) {
		printf("This file has not been opened.\n");
		return NULL;
	} else if (target_directory->mode != RDONLY) {
		printf("Permission Denied.\n");
		return NULL;
	}
	// now, after confirming that this directory is open, we need to read the directory under this directory
	char *dir_ptr_char = (char *)malloc(sizeof(directory_entry));
	//directory_entry* dir_ptr = new directory_entry;
	int counter = open_directory_tracker[dirfd]; // maybe we don't need it
	int block_index = target_directory->block_index; // block index tells us the next datablock to read from
	int block_offset = target_directory->block_offset; // block offset tells us which data block
	int byte_offset = target_directory->byte_offset; // byte offset tells us which entry under this directory
	inode* target_directory_inode = disk_inode_region[target_directory->inode_entry];
	int data_offset = sb->data_offset;
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	int num_of_directory_entries = target_directory_inode->size / (BLOCK_SIZE * sizeof(directory_entry)); // get the number of entries under this directory
	// if the byte_offset is 0, that means we need to give the user the current directory, "."
	if (counter >= target_directory_inode->size / BLOCK_SIZE) {
		return NULL;
	}

	if (counter == 0) {
		directory_entry* dir_ptr = (directory_entry *)dir_ptr_char;
		dir_ptr->inode_entry = target_directory->inode_entry;
		std::strcpy(dir_ptr->file_name, ".");
		counter ++;
		open_directory_tracker[dirfd] = counter;
		byte_offset += sizeof(directory_entry); // increment the byte_offset by the sizeof directory_entry.
		target_directory->byte_offset = byte_offset; // update the offset
		return dir_ptr;
	} else if (counter == 1) {
		int parent_inode_index = target_directory_inode->parent; // get the parent's inode index
		directory_entry* dir_ptr = (directory_entry *)dir_ptr_char;
		dir_ptr->inode_entry = parent_inode_index;
		std::strcpy(dir_ptr->file_name, "..");
		counter ++;
		open_directory_tracker[dirfd] = counter;
		byte_offset += sizeof(directory_entry); // increment the byte_offset by the sizeof directory_entry.
		target_directory->byte_offset = byte_offset; // update the offset
		return dir_ptr;
	} else {
		// Third call, Fourth call, etc.
		// use lseek and read.
		// go to the corresponding directory entry.
		if (byte_offset < BLOCK_SIZE) {
			byte_offset += sizeof(directory_entry); // increment the byte_offset by the sizeof directory_entry.
			target_directory->byte_offset = byte_offset; // update the offset
			open_file_table[dirfd] = target_directory; // update the open file table, maybe this step is unnecessary
			if (lseek(disk, data_region_starting_addr + BLOCK_SIZE * block_index + byte_offset - sizeof(directory_entry), SEEK_SET) == FAIL) {
				printf("[lseek in f_readdir] Fails");
				return NULL;
			}
		} 
		else if (byte_offset >= BLOCK_SIZE) {
			block_offset ++;
			target_directory->block_offset = block_offset; // increment the block_offset by 1
			target_directory->byte_offset = 0; // reset the byte_offset to 0
			target_directory->block_index = get_index_by_offset(target_directory_inode, block_offset); // get the block index.
			open_file_table[dirfd] = target_directory; // update the open file table, maybe this step is unnecessary
			if (lseek(disk, data_region_starting_addr + BLOCK_SIZE * block_index + byte_offset, SEEK_SET) == FAIL) {
				printf("[lseek in f_readdir] Fails");
				return NULL;
			}
		}
		counter++;
		open_directory_tracker[dirfd] = counter;
		read(disk, dir_ptr_char, sizeof(directory_entry)); // read it into a directory entry buffer.
		directory_entry* dir_ptr = (directory_entry *)dir_ptr_char;
		// now update the open file table
		// if the byte_offset is less than the block size, we just need to update the byte offset.

		return dir_ptr;
	}
	// if it still does not return, something wrong happens.
	return NULL;
}

// f_closedir just needs to call f_close because we treat the directory as a file.
// if f_close works, f_closedir should also works.
int f_closedir(int dirfd) {
	return f_close(dirfd);
}

// f_mkdir: 
// if the directory already exists return Fail. Otherwise create a new directory.
// Not Tested yet. Potential buggy.
int f_mkdir(string path, int mode) {
	if (path == "." || path == ".." || path == "/") {
		cout << "Invalid directory name." << endl;
		return FAIL;
	} else if (mode != EXEONLY && mode != WRONLY && mode != RDONLY) {
		cout << "Illege permission mode." << endl;
		return FAIL;
	}
	//parse the path to know the filename
	vector<string> path_list = split(path, '/');
	for (int i = 0; i < path_list.size(); i++)
	{
		cout << "This element is " << path_list[i] << endl;
	}
	int dir_node = sb->root;
	int parent_node; // to get the parent directory inode of the 
	// check if the directory has already existed.
	for (int i = 0; i < path_list.size(); i++){
		parent_node = dir_node;
		dir_node = traverse_directory(dir_node, path_list[i]);
		if (dir_node != FAIL) {
			cout << "The directory has already existed." << endl;
			return FAIL;
		}
	}
	// if it does not exist, create the new directory.
	// using create_file by assuming this function is correct.
	if (create_file(path_list[path_list.size()-1], parent_node, DIRECTORY_FILE, mode) == EXIT_FAILURE) {
		cout << "Fail to create the new directory." << endl;
		return FAIL;
	}
	// if created successfully, return Success.
	return SUCCESS;
}

// f_rmdir: using f_remove to recursively remove the given directory. It is very slow...
// Need to be tested. Potential very buggy.
int f_rmdir(string filepath) {
	//parse the path to know the filename
	vector<string> path_list = split(filepath, '/');
	for (int i = 0; i < path_list.size(); i++)
	{
		cout << "This element is " << path_list[i] << endl;
	}
	int dir_node = sb->root;
	// check if the directory has already existed.
	for (int i = 0; i < path_list.size(); i++){
		dir_node = traverse_directory(dir_node, path_list[i]);
		if (dir_node == FAIL) {
			cout << "The directory does not exist." << endl;
			return FAIL;
		}
	}
	// now we can delete the directory
	inode* directory_inode = disk_inode_region[dir_node]; // get the directory inode
	int permission = directory_inode->permission;
	unsigned long file_size = directory_inode->size;
	int data_offset = sb->data_offset;
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	int num_of_directory_entries = file_size / (BLOCK_SIZE * sizeof(directory_entry)); // calculate the number of directory entries. Hope it is correct.
	size_t remaining_size = file_size;
	// if the directory permission is not rx and rwx, you don't have the permission to delete the directory
	if (permission != WRONLY + EXEONLY && permission != RDONLY + WRONLY + EXEONLY) {
		cout << "[f_rmdir] Permission Denied." << endl;
		return FAIL;
	}
	// iterate through all the directory entries to delete them
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		int data_block_index = directory_inode->dblocks[i];
		// one data block can have 16 directory entries
		size_t directories_starting_region = data_region_starting_addr + BLOCK_SIZE * data_block_index;
		for (int j = 0; j < BLOCK_SIZE / sizeof(directory_entry); j++)
		{
			// may not work! Logic seems to be fine.
			lseek(disk, directories_starting_region + j * sizeof(directory_entry), SEEK_SET);
			//char *entry_char = (char *)malloc(sizeof(directory_entry));
			char entry_char[sizeof(directory_entry)];
			read(disk, entry_char, sizeof(directory_entry));
			directory_entry *entry = (directory_entry *)entry_char;
			//directory_entry *entry = (directory_entry *)disk_buffer + directories_starting_region + j * sizeof(directory_entry); // get each directory entry
			// check if it is a directory or a file.
			char *name = entry->file_name;
			int inode_index_of_file = entry->inode_entry;
			if (disk_inode_region[inode_index_of_file]->type == DIRECTORY_FILE) {
				string directory_path_string = filepath + string(name);
				f_rmdir(directory_path_string);
			} else {
				string file_path_string = filepath + string(name);
				f_remove(file_path_string);
			}
			remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
			// if the remaining size is <= 64 bytes, which means it is an emptry directory, delete it self.
			// hopefully it works..
			if (remaining_size <= sizeof(directory_entry) * 2) {
				return f_remove(filepath);
			}
			if (remaining_size <= 0)
			{
				return SUCCESS;
			}
		}
	}
	// if the remaining_size is not 0, we still need to look into other data blocks.
	// Then, look into the single indirect data blocks
	size_t ptr_addr;
	for (int i = 0; i < N_IBLOCKS; i++)
	{
		ptr_addr = data_region_starting_addr + BLOCK_SIZE * directory_inode->iblocks[i];
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				// check if it is a directory or a file.
				char *name = entry->file_name;
				int inode_index_of_file = entry->inode_entry;
				if (disk_inode_region[inode_index_of_file]->type == DIRECTORY_FILE) {
					string directory_path_string = filepath + string(name);
					f_rmdir(directory_path_string);
				} else {
					string file_path_string = filepath + string(name);
					f_remove(file_path_string);
				}
				remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
				if (remaining_size <= sizeof(directory_entry) * 2) {
					return f_remove(filepath);
				}
				if (remaining_size <= 0)
				{
					return SUCCESS;
				}
			}
		}
	}
	// Next, look into the double indirect data blocks
	size_t i2block_ptr_addr = data_region_starting_addr + directory_inode->i2block * BLOCK_SIZE;
	char indirect_data_blocks_chars[BLOCK_SIZE];
	bzero(indirect_data_blocks_chars, BLOCK_SIZE);
	lseek(disk, i2block_ptr_addr, SEEK_SET);
	read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
	//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
	int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
	// loop through 128 indirect data blocks
	for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
	{
		size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, idblock_ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				// check if it is a directory or a file.
				char *name = entry->file_name;
				int inode_index_of_file = entry->inode_entry;
				if (disk_inode_region[inode_index_of_file]->type == DIRECTORY_FILE) {
					string directory_path_string = filepath + string(name);
					f_rmdir(directory_path_string);
				} else {
					string file_path_string = filepath + string(name);
					f_remove(file_path_string);
				}
				remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
				if (remaining_size <= sizeof(directory_entry) * 2) {
					return f_remove(filepath);
				}
				if (remaining_size <= 0)
				{
					return SUCCESS;
				}
			}
		}
	}

	// Finally, look into the triple indirect data blocks
	size_t i3block_ptr_addr = data_region_starting_addr + directory_inode->i3block * BLOCK_SIZE;
	char i2blocks_char[BLOCK_SIZE];
	bzero(i2blocks_char, BLOCK_SIZE);
	lseek(disk, i3block_ptr_addr, BLOCK_SIZE);
	read(disk, i2blocks_char, BLOCK_SIZE);
	//memcpy(i2blocks_char, disk_buffer + i3block_ptr_addr, BLOCK_SIZE);
	int *i2blocks = (int *)i2blocks_char;
	// first, loop through 128 doubly indirect data blocks
	for (int o = 0; o < BLOCK_SIZE / sizeof(int); o++)
	{
		size_t i2block_ptr_addr = data_region_starting_addr + i2blocks[o] * BLOCK_SIZE;
		char indirect_data_blocks_chars[BLOCK_SIZE];
		bzero(indirect_data_blocks_chars, BLOCK_SIZE);
		lseek(disk, i2block_ptr_addr, BLOCK_SIZE);
		read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
		//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
		int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
		// loop through 128 indirect data blocks
		for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
		{
			size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
			char data_blocks_char[BLOCK_SIZE];
			bzero(data_blocks_char, BLOCK_SIZE);
			lseek(disk, idblock_ptr_addr, BLOCK_SIZE);
			read(disk, data_blocks_char, BLOCK_SIZE);
			//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
			int *direct_data_block = (int *)data_blocks_char;
			// loop through 128 direct data blocks pointers.
			for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
			{
				size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
				// loop through 16 directory entries.
				for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
				{
					lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
					char entry_char[sizeof(directory_entry)];
					read(disk, entry_char, sizeof(directory_entry));
					directory_entry *entry = (directory_entry *)entry_char;
					//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
					// check if it is a directory or a file.
					char *name = entry->file_name;
					int inode_index_of_file = entry->inode_entry;
					if (disk_inode_region[inode_index_of_file]->type == DIRECTORY_FILE) {
						string directory_path_string = filepath + string(name);
						f_rmdir(directory_path_string);
					} else {
						string file_path_string = filepath + string(name);
						f_remove(file_path_string);
					}
					remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
					if (remaining_size <= sizeof(directory_entry) * 2) {
						return f_remove(filepath);
					}
					if (remaining_size <= 0)
					{
						return SUCCESS;
					}
				}
			}
		}
	}
	// it still does not return, something is wrong.
	return FAIL;
}

//http://ysonggit.github.io/coding/2014/12/16/split-a-string-using-c.html for split
vector<string> split(const string &s, char delim)
{
	stringstream ss(s);
	string item;
	vector<string> tokens;
	while (getline(ss, item, delim))
	{
		if (item != "")
			tokens.push_back(item);
	}
	return tokens;
}

int add_to_file_table(int inode_num, inode *f_node,int mode)
{
	//get the inode from inode region
	int i;
	for (i = 0; i < MAX_OPEN_FILE; i++)
	{
		if (open_file_table[i]->inode_entry == -1)
		{
			open_file_table[i]->inode_entry = inode_num;
			open_file_table[i]->block_index = f_node->dblocks[0];
			open_file_table[i]->block_offset = 1;
			open_file_table[i]->byte_offset = 0;
			open_file_table[i]->mode = mode;
			break;
		}
	}
	return i;
}

//find and delete the last entry and return its directory_entry node.
directory_entry* deal_last_entry(int dirinode_index, const char* filename) {
	//find the last entry in the directory, if the last one is the deleted one, return null
	inode* cur = disk_inode_region[dirinode_index];
	size_t size = cur->size - sizeof(directory_entry);
	bool flag = false;
	int last_block_offset = size/BLOCK_SIZE + 1;
	int last_byte_offset = size%BLOCK_SIZE;
	//test printing
	printf("test deal last entry print ****************************\n");
	printf("last_block_offset %d\n",last_block_offset);
	printf("last_byte offset %d\n",last_byte_offset);

	//get the data block of the last entry
	int last_block_index = get_index_by_offset(cur,last_block_offset);
	printf("last_block_index %d\n",last_block_index);
	//get the block of entry from disk
	char* last_entry_block = (char*)(malloc(BLOCK_SIZE));
	size_t last_entry_addr = BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + last_block_index * BLOCK_SIZE;
	lseek(disk,last_entry_addr,SEEK_SET);
	read(disk,last_entry_block,BLOCK_SIZE);

	//use block to get the specific entry 
	int last_entry_index = last_byte_offset/(sizeof(directory_entry));
	printf("last_entry_index %d\n",last_entry_index);
	directory_entry* entry_table = (directory_entry*)last_entry_block;
	printf("last_entry filename is %s\n",entry_table[last_entry_index].file_name);
	printf("last_entry inode is %d\n",entry_table[last_entry_index].inode_entry);
	//create a return directory entry
	directory_entry* return_node = (directory_entry*)(malloc(sizeof(directory_entry)));
	strcpy(return_node->file_name, entry_table[last_entry_index].file_name);
	return_node->inode_entry = entry_table[last_entry_index].inode_entry;
	//if it is the last element, set the flag and return null
	if(strcmp(entry_table[last_entry_index].file_name,filename) == 0) {
		flag = true;
		printf("It is the last element !!\n");
	}
	strcpy(entry_table[last_entry_index].file_name,"");
	entry_table[last_entry_index].inode_entry = 0;
	//write back to disk
	lseek(disk,last_entry_addr,SEEK_SET);
	write(disk,last_entry_block,BLOCK_SIZE);
	//change the directory file size
	cur->size -= sizeof(directory_entry);
	//write back to inode region in disk
	lseek(disk, BOOT_SIZE + SUPER_SIZE + dirinode_index * sizeof(inode), SEEK_SET);
	write(disk, cur, sizeof(inode));
	//need to update the openfile table

	//return the return_node
	if(flag)
		return NULL;
	else
		return return_node;

}

//similiar to traverse_dir ,but swap the last entry in directory with the entry we want to remove
//first one need to find the specific entry we want to remove in the parent directory
//we then find and delete the last entry in the parent directory
//use the return value( a directory entry struct) to replace the current entry
//if succeed, return 0, else return -1
int swap_entry(int dirinode_index, string filename)
{
	//bool IsExist = false;
	//size_t size_of_disk = 6666666666;				  // IT MUST BE CHANGED LATER.
	//char *disk_buffer = (char *)malloc(size_of_disk); // IMPORTANT: hard coded buffer that contains the disk image data!!!, This needs to be changed later.
	// first, check if the given inode has an entry in the disk inode region (i.e., Check if the directory exists or not)
	inode *direct = disk_inode_region[dirinode_index]; // get the directory
	if (direct->nlink == 0)
	{
		printf("This directory does not exist.\n");
		return FAIL;
	}
	// then, check if the given inode is a directory inode or not
	if (direct->type != DIRECTORY_FILE)
	{
		printf("This is not a directory.\n");
		return FAIL;
	}
	// now lets check the data region to check if the given filename exists or not
	// first, look into the direct data blocks
	size_t remaining_size = direct->size; // get the size of the directory, and says it is the remaining size.
	printf("swap entry testing print! &&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
	printf("remaining_size is %ld\n",remaining_size);
	cout << "the file name is " << filename << endl;
	int data_offset = sb->data_offset;	// get the data offset from the super block
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		int data_block_index = direct->dblocks[i];
		// one data block can have 16 directory entries
		size_t directories_starting_region = data_region_starting_addr + BLOCK_SIZE * data_block_index;
		for (int j = 0; j < BLOCK_SIZE / sizeof(directory_entry); j++)
		{
			// may not work! Logic seems to be fine.
			lseek(disk, directories_starting_region + j * sizeof(directory_entry), SEEK_SET);
			//char *entry_char = (char *)malloc(sizeof(directory_entry));
			char entry_char[sizeof(directory_entry)];
			read(disk, entry_char, sizeof(directory_entry));
			directory_entry *entry = (directory_entry *)entry_char;
			//directory_entry *entry = (directory_entry *)disk_buffer + directories_starting_region + j * sizeof(directory_entry); // get each directory entry
			if (string(entry->file_name) == filename)
			{
				directory_entry* input = deal_last_entry(dirinode_index,filename.c_str());
				if(input != NULL) {
					lseek(disk, directories_starting_region + j * sizeof(directory_entry), SEEK_SET);
					write(disk,input,sizeof(directory_entry));
				}
				return SUCCESS;
			}
			remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
			if (remaining_size <= 0)
			{
				return FAIL;
			}
		}
	}
	// if the remaining_size is not 0, we still need to look into other data blocks.
	// Then, look into the single indirect data blocks
	size_t ptr_addr;
	for (int i = 0; i < N_IBLOCKS; i++)
	{
		ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct->iblocks[i];
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				if (string(entry->file_name) == filename)
				{
					directory_entry* input = deal_last_entry(dirinode_index,filename.c_str());
					if(input != NULL) {
						lseek(disk,dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
						write(disk,input,sizeof(directory_entry));
					}
					return SUCCESS;
				}
				remaining_size -= sizeof(directory_entry);
				if (remaining_size <= 0)
				{
					return FAIL;
				}
			}
		}
	}
	// Next, look into the double indirect data blocks
	size_t i2block_ptr_addr = data_region_starting_addr + direct->i2block * BLOCK_SIZE;
	char indirect_data_blocks_chars[BLOCK_SIZE];
	bzero(indirect_data_blocks_chars, BLOCK_SIZE);
	lseek(disk, i2block_ptr_addr, SEEK_SET);
	read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
	//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
	int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
	// loop through 128 indirect data blocks
	for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
	{
		size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, idblock_ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				if (string(entry->file_name) == filename)
				{
					directory_entry* input = deal_last_entry(dirinode_index,filename.c_str());
					if(input != NULL) {
						lseek(disk,dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
						write(disk,input,sizeof(directory_entry));			
					}		
					return SUCCESS;
				}
				remaining_size -= sizeof(directory_entry);
				if (remaining_size <= 0)
				{
					return FAIL;
				}
			}
		}
	}

	// Finally, look into the triple indirect data blocks
	size_t i3block_ptr_addr = data_region_starting_addr + direct->i3block * BLOCK_SIZE;
	char i2blocks_char[BLOCK_SIZE];
	bzero(i2blocks_char, BLOCK_SIZE);
	lseek(disk, i3block_ptr_addr, BLOCK_SIZE);
	read(disk, i2blocks_char, BLOCK_SIZE);
	//memcpy(i2blocks_char, disk_buffer + i3block_ptr_addr, BLOCK_SIZE);
	int *i2blocks = (int *)i2blocks_char;
	// first, loop through 128 doubly indirect data blocks
	for (int o = 0; o < BLOCK_SIZE / sizeof(int); o++)
	{
		size_t i2block_ptr_addr = data_region_starting_addr + i2blocks[o] * BLOCK_SIZE;
		char indirect_data_blocks_chars[BLOCK_SIZE];
		bzero(indirect_data_blocks_chars, BLOCK_SIZE);
		lseek(disk, i2block_ptr_addr, BLOCK_SIZE);
		read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
		//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
		int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
		// loop through 128 indirect data blocks
		for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
		{
			size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
			char data_blocks_char[BLOCK_SIZE];
			bzero(data_blocks_char, BLOCK_SIZE);
			lseek(disk, idblock_ptr_addr, BLOCK_SIZE);
			read(disk, data_blocks_char, BLOCK_SIZE);
			//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
			int *direct_data_block = (int *)data_blocks_char;
			// loop through 128 direct data blocks pointers.
			for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
			{
				size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
				// loop through 16 directory entries.
				for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
				{
					lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
					char entry_char[sizeof(directory_entry)];
					read(disk, entry_char, sizeof(directory_entry));
					directory_entry *entry = (directory_entry *)entry_char;
					//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
					if (string(entry->file_name) == filename)
					{
						directory_entry* input = deal_last_entry(dirinode_index,filename.c_str());
						if(input != NULL) {
							lseek(disk,dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
							write(disk,input,sizeof(directory_entry));	
						}					
						return SUCCESS;
					}
					remaining_size -= sizeof(directory_entry);
					if (remaining_size <= 0)
					{
						return FAIL;
					}
				}
			}
		}
	}
	// if it still doesn't return, return a -1.
	return FAIL;
}

// disk_buffer needs to be changed... I need the disk image to be in a buffer to get its data region.
// use read and lseek
// Maybe buggy...... Needs to be tested.
int traverse_dir(int dirinode_index, string filename, bool isLast)
{
	//bool IsExist = false;
	//size_t size_of_disk = 6666666666;				  // IT MUST BE CHANGED LATER.
	//char *disk_buffer = (char *)malloc(size_of_disk); // IMPORTANT: hard coded buffer that contains the disk image data!!!, This needs to be changed later.
	// first, check if the given inode has an entry in the disk inode region (i.e., Check if the directory exists or not)
	inode *direct = disk_inode_region[dirinode_index]; // get the directory
	if (direct->nlink == 0)
	{
		printf("This directory does not exist.\n");
		return FAIL;
	}
	// then, check if the given inode is a directory inode or not
	if (!isLast && direct->type != DIRECTORY_FILE)
	{
		printf("This is not a directory.\n");
		return FAIL;
	}
	// now lets check the data region to check if the given filename exists or not
	// first, look into the direct data blocks
	size_t remaining_size = direct->size; // get the size of the directory, and says it is the remaining size.
	int data_offset = sb->data_offset;	// get the data offset from the super block
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		int data_block_index = direct->dblocks[i];
		// one data block can have 16 directory entries
		size_t directories_starting_region = data_region_starting_addr + BLOCK_SIZE * data_block_index;
		for (int j = 0; j < BLOCK_SIZE / sizeof(directory_entry); j++)
		{
			// may not work! Logic seems to be fine.
			lseek(disk, directories_starting_region + j * sizeof(directory_entry), SEEK_SET);
			//char *entry_char = (char *)malloc(sizeof(directory_entry));
			char entry_char[sizeof(directory_entry)];
			read(disk, entry_char, sizeof(directory_entry));
			directory_entry *entry = (directory_entry *)entry_char;
			//directory_entry *entry = (directory_entry *)disk_buffer + directories_starting_region + j * sizeof(directory_entry); // get each directory entry
			if (string(entry->file_name) == filename)
			{
				return entry->inode_entry;
			}
			remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
			if (remaining_size <= 0)
			{
				return FAIL;
			}
		}
	}
	// if the remaining_size is not 0, we still need to look into other data blocks.
	// Then, look into the single indirect data blocks
	size_t ptr_addr;
	for (int i = 0; i < N_IBLOCKS; i++)
	{
		ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct->iblocks[i];
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				if (string(entry->file_name) == filename)
				{
					return entry->inode_entry;
				}
				remaining_size -= sizeof(directory_entry);
				if (remaining_size <= 0)
				{
					return FAIL;
				}
			}
		}
	}
	// Next, look into the double indirect data blocks
	size_t i2block_ptr_addr = data_region_starting_addr + direct->i2block * BLOCK_SIZE;
	char indirect_data_blocks_chars[BLOCK_SIZE];
	bzero(indirect_data_blocks_chars, BLOCK_SIZE);
	lseek(disk, i2block_ptr_addr, SEEK_SET);
	read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
	//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
	int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
	// loop through 128 indirect data blocks
	for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
	{
		size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
		char data_blocks_char[BLOCK_SIZE];
		bzero(data_blocks_char, BLOCK_SIZE);
		lseek(disk, idblock_ptr_addr, SEEK_SET);
		read(disk, data_blocks_char, BLOCK_SIZE);
		//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
		int *direct_data_block = (int *)data_blocks_char;
		// loop through 128 direct data blocks pointers.
		for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
		{
			size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
			// loop through 16 directory entries.
			for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
			{
				lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
				char entry_char[sizeof(directory_entry)];
				read(disk, entry_char, sizeof(directory_entry));
				directory_entry *entry = (directory_entry *)entry_char;
				//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
				if (string(entry->file_name) == filename)
				{
					return entry->inode_entry;
				}
				remaining_size -= sizeof(directory_entry);
				if (remaining_size <= 0)
				{
					return FAIL;
				}
			}
		}
	}

	// Finally, look into the triple indirect data blocks
	size_t i3block_ptr_addr = data_region_starting_addr + direct->i3block * BLOCK_SIZE;
	char i2blocks_char[BLOCK_SIZE];
	bzero(i2blocks_char, BLOCK_SIZE);
	lseek(disk, i3block_ptr_addr, BLOCK_SIZE);
	read(disk, i2blocks_char, BLOCK_SIZE);
	//memcpy(i2blocks_char, disk_buffer + i3block_ptr_addr, BLOCK_SIZE);
	int *i2blocks = (int *)i2blocks_char;
	// first, loop through 128 doubly indirect data blocks
	for (int o = 0; o < BLOCK_SIZE / sizeof(int); o++)
	{
		size_t i2block_ptr_addr = data_region_starting_addr + i2blocks[o] * BLOCK_SIZE;
		char indirect_data_blocks_chars[BLOCK_SIZE];
		bzero(indirect_data_blocks_chars, BLOCK_SIZE);
		lseek(disk, i2block_ptr_addr, BLOCK_SIZE);
		read(disk, indirect_data_blocks_chars, BLOCK_SIZE);
		//memcpy(indirect_data_blocks_chars, disk_buffer + i2block_ptr_addr, BLOCK_SIZE);
		int *indirect_data_blocks = (int *)indirect_data_blocks_chars;
		// loop through 128 indirect data blocks
		for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++)
		{
			size_t idblock_ptr_addr = data_region_starting_addr + indirect_data_blocks[i] * BLOCK_SIZE;
			char data_blocks_char[BLOCK_SIZE];
			bzero(data_blocks_char, BLOCK_SIZE);
			lseek(disk, idblock_ptr_addr, BLOCK_SIZE);
			read(disk, data_blocks_char, BLOCK_SIZE);
			//memcpy(data_blocks_char, disk_buffer + idblock_ptr_addr, BLOCK_SIZE);
			int *direct_data_block = (int *)data_blocks_char;
			// loop through 128 direct data blocks pointers.
			for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
			{
				size_t dBlock_ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct_data_block[j];
				// loop through 16 directory entries.
				for (int k = 0; k < BLOCK_SIZE / sizeof(directory_entry); k++)
				{
					lseek(disk, dBlock_ptr_addr + k * sizeof(directory_entry), SEEK_SET);
					char entry_char[sizeof(directory_entry)];
					read(disk, entry_char, sizeof(directory_entry));
					directory_entry *entry = (directory_entry *)entry_char;
					//directory_entry *entry = (directory_entry *)disk_buffer + dBlock_ptr_addr + k * sizeof(directory_entry);
					if (string(entry->file_name) == filename)
					{
						return entry->inode_entry;
					}
					remaining_size -= sizeof(directory_entry);
					if (remaining_size <= 0)
					{
						return FAIL;
					}
				}
			}
		}
	}
	// if it still doesn't return, return a -1.
	return FAIL;
}

int remove_p_dir_entry(char* file_name,int parent_index) {
	return swap_entry(parent_index,string(file_name));

}
int f_remove(const string path) {
	vector<string> path_list = split(path, '/');
	for (int i = 0; i < path_list.size(); i++)
	{
		cout << "This element is " << path_list[i] << endl;
	}

	int dir_node = sb->root;
	bool isLast = false;
	int temp = -1;
	int counter = 0;
	for (counter = 0; counter < path_list.size(); counter++)
	{
		// to see if it is the last thing we need to find
		if (counter == path_list.size() - 1) {
			isLast = true;
		}
		temp = dir_node;
		dir_node = traverse_dir(dir_node, path_list[counter], isLast); //and need to consider permission problem
		//not that simple need to consider restrict_mode
		if (dir_node == -1)
		{
			cout << "The file path is incorrect" << endl;
			// return FAIL;
			break;
		}
	}
	if (dir_node == -1) {
		//if path is wrong or file does not exist, return fail
		return EXIT_FAILURE;
	}
	else {
		for (int k = 0; k < MAX_OPEN_FILE; k++) {
			if (open_file_table[k]->inode_entry == dir_node) {
				printf("please close the file before removing it\n");
				return EXIT_FAILURE;  //file can not be removed if it is open
			}
		}
	}
	//if it exists, get the inode pointer
	inode *target = disk_inode_region[dir_node];
	//check permission
	//printf("the permission is %d\n",target->permission);
	if(target->permission != (EXEONLY + WRONLY) && target->permission != (EXEONLY + WRONLY + RDONLY)) {
		printf("Permission denied by our file system!\n");
		return EXIT_FAILURE;
	}
	//change the parent directory
	inode *parent = disk_inode_region[target->parent];
	if(remove_p_dir_entry(target->file_name,target->parent) < 0) {
		printf("remove last entry fails!\n");
		return EXIT_FAILURE;
	}
	//test printing before clean
	printf("This is f_remove test printing!########################\n");
	print_superblock(sb);
	print_inode(target,dir_node,1);

	//superblock not be updated when create file
	clean_file(target);
	clean_inode(target, dir_node);
	update_sb();

	//test printing
	print_superblock(sb);
	print_inode(target,dir_node,1);
	printf("next free inode is %d\n",target->next_inode);

	return SUCCESS;

}

int f_open(const string restrict_path, const string restrict_mode)
{
	//we need to check mode here to avoid invalid mode
	if (restrict_mode != "r" && restrict_mode != "w" && restrict_mode != "a")
	{
		cout << "The open mode is incorrect, please input \"r\" or \"w\" or \"a\"" << endl;
		return FAIL;
	}
	//parse the path to know the filename
	vector<string> path_list = split(restrict_path, '/');
	for (int i = 0; i < path_list.size(); i++)
	{
		cout << "This element is " << path_list[i] << endl;
	}

	//we can copy the inode region to memory! so search through the path to see whether the file is in the right position -- in mount
	//zhanpeng is implementing function traverse_dir(int dir_node, string filename)
	//first get the root directory inode index -- where we initialize all of these?
	//how can we go into the disk to find the specific block if disk image is not on memory
	int dir_node = sb->root;
	int temp = -1;
	bool isLast = false;
	int counter = 0;
	for (counter = 0; counter < path_list.size(); counter++)
	{
		// to see if it is the last thing we need to find
		if (counter == path_list.size() - 1) {
			isLast = true;
		}
		temp = dir_node;
		dir_node = traverse_dir(dir_node, path_list[counter], isLast); //and need to consider permission problem
		//not that simple need to consider restrict_mode
		if (dir_node == -1)
		{
			cout << "The file path is incorrect" << endl;
			// return FAIL;
			break;
		}
	}
	//printf("the dir_node of this file is %d\n",dir_node);
	if (dir_node == -1) {
		if (counter != path_list.size() - 1) {
			return EXIT_FAILURE;
		}
		else if (restrict_mode == "a" || restrict_mode == "w") {
			dir_node = create_file(path_list[counter], temp, NORMAL_FILE, RDONLY + WRONLY + EXEONLY); // when mode is "w" or "a", create a file if the file doesn't exist
			for (int k = 0; k < MAX_OPEN_FILE; k++) {
				if (open_file_table[k]->inode_entry == dir_node) {
					return k;
				}
			}
		}
		else {
			return EXIT_FAILURE;
		}
	}
	else {
		for (int k = 0; k < MAX_OPEN_FILE; k++) {
			if (open_file_table[k]->inode_entry == dir_node) {
				return EXIT_FAILURE;  // file has already been opened
			}
		}
	}
	//if we get out of loop, it should get the target file's inode then we set up the open file table
	/*set up open file table：
		1.find the index of this file in open file table -- return value
		2.find inode index
		3.find the first data block index
		4.set the rest
	*/
	inode *target = disk_inode_region[dir_node];
	//if not in the file table, add an new element in it
	int result;
	if(restrict_mode == "r")
		result = add_to_file_table(dir_node, target,RDONLY);
	else
		result = add_to_file_table(dir_node, target,WRONLY);
	//print_file_table();
	//we also need to deal with open_file_table, have a function to add and remove element in open file table

	// if we open a existed file with "w", we need to update its inode information -- one problem, if not change correspond data blocks, those data blocks will be lost
	if (restrict_mode == "w") {
		//first deal with data blocks
		clean_file(target);
		//then update the superblock in disk image
		update_sb();
		disk_inode_region[dir_node]->size = 0;
		for (int i = 0; i < N_DBLOCKS; i++) {
			(disk_inode_region[dir_node]->dblocks)[i] = -1;
		}
		for (int i = 0; i < N_IBLOCKS; i++) {
			(disk_inode_region[dir_node]->iblocks)[i] = -1;
		}
		disk_inode_region[dir_node]->i2block = -1;
		disk_inode_region[dir_node]->i3block = -1;
		// write the change of inode to disks
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + dir_node * sizeof(inode), SEEK_SET);
		write(disk, disk_inode_region[dir_node], sizeof(inode));
	}
	else {
		/*  We need to update the file indicator in the open file table for mode "a" if the file exists */
		if(restrict_mode == "a") {
			//offset start from 1
			int block_offset = target->size/BLOCK_SIZE + 1;
			int byte_offset = target->size%BLOCK_SIZE;
			int block_index = get_index_by_offset(target,block_offset);
			printf("block_index we have is %d\n",block_index);
			open_file_table[result]->block_index = block_index;
			open_file_table[result]->block_offset = block_offset;
			open_file_table[result]->byte_offset = byte_offset;
			print_file_status(result);
		}
	}

	return result;
}

// f_read: Not tested yet. Maybe buggy.
size_t f_read(void *restrict_ptr, size_t size, size_t nitems, int fd) {
	// Check if it is a valid file descriptor. Then, checking if the file is open or not.
	// Next, check if the user has the right permission to read the file.
	file_node *target_file = open_file_table[fd]; // get the file
	if (fd < 0 || fd > MAX_OPEN_FILE) {
		printf("Invalid file descriptor.\n");
		return FAIL;
	} else if (target_file->inode_entry == -1) {
		printf("This file has not been opened.\n");
		return FAIL;
	} else if (target_file->mode != RDONLY) {
		printf("Permission Denied!!!!!\n");
		return FAIL;
	} else if (size <= 0 || nitems <=0) {
		printf("Invalid size or nitems.\n");
		return FAIL;
	}
	/*
	logic:
		Check every time if the remaining size is 0, if it is zero, break the loop and return the requested size to the user.
		1. Go to the first data block by using block index, and start reading the first data block with the block offset
		2. using lseek and read to read the first data block into the buffer (restrict_ptr), reduce the remaining size by the bytes
		3. if we need to read more data blocks, update the byte offset to 0, increment block offset by 1, use get_index_by_offset to
		   get the block_index of the next data block, update the block index.
		4. lseek and read the next data blocks. Append it to the buffer + BLOCK_SIZE. Reduce the remaining size by the amount of bytes
		   that has been read into the buffer.
		5. if the last data block is less than BLOCK_SIZE (i.e. if 0< remaining size < 512), we need to update the block index, block_offset,
		   byte_offset of the target_file. Finally, update the open file table.
	*/
	size_t size_requested = size * nitems; // set the size asked by the user
	size_t remaining_size = size_requested; // set the remaining size.
	int block_index = target_file->block_index;
	int block_offset = target_file->block_offset;
	int byte_offset = target_file->byte_offset;
	int inode_index = target_file->inode_entry;
	inode* file_inode = disk_inode_region[inode_index];
	int data_offset = sb->data_offset;
	int tmp = byte_offset;
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	int i = 0; // keep track of where to append the buffer.
	void* tmp_ptr = restrict_ptr;
	int file_size = file_inode->size; // get the file size.
	// potential buggy while loop!!!!!!!!!!!!!!!!!!!!!!
	print_file_status(fd);
	printf("This is f_read test printing!**************************\n");
	printf("remaining_size is %d\n",remaining_size);
	// if the file size is less than the size requested by the user
	if (file_size <= size_requested) {
		remaining_size = file_size;
		size_requested = file_size;
	}
	while (remaining_size >0) {
		if (lseek(disk, data_region_starting_addr + BLOCK_SIZE * block_index + byte_offset, SEEK_SET) == FAIL) {
			printf("[lseek in f_read] Fails");
			return FAIL;
		}
		if (byte_offset != 0) {
			printf("the byte offset is not 0\n");
			int bytes_read = BLOCK_SIZE - byte_offset;
			read(disk, restrict_ptr, bytes_read);
			byte_offset = 0; // reset the byte offset
			block_offset ++; // increment the block offset by 1
			block_index = get_index_by_offset(file_inode, block_offset); // use the func to get the next block index.
			remaining_size -= bytes_read; // reduce the remaining size by the amount of bytes that has been read.
		} else {
			// if 0 < remaining_size <= 512, i.e. it is a last block, we need to set the block index, block offset,
			// and byte offset of the target_file, and update the open file table.
			if (0 < remaining_size && remaining_size <= BLOCK_SIZE) {
				tmp_ptr  = (void*)((char*)restrict_ptr + tmp + BLOCK_SIZE * i);
				int test = read(disk, tmp_ptr, remaining_size);
				printf("the test tmp_ptr now is %s and %d\n",(char*)tmp_ptr,test);
				byte_offset = remaining_size; // since the last block has the byte offset to be 0, the new byte offset will be remaining size - 0
				target_file->byte_offset = byte_offset; // update byte_offset
				target_file->block_offset = block_offset; // update block_offset
				target_file->block_index = block_index; // update block_index
				open_file_table[fd] = target_file; // put the new file node into the open file table.
				//testing print
				print_file_status(fd);
				return size_requested;
			} else {
				tmp_ptr  = (void*)((char*)restrict_ptr + tmp + BLOCK_SIZE * i);
				int test = read(disk, tmp_ptr, BLOCK_SIZE);
				printf("the test tmp_ptr now is %s and %d\n",(char*)tmp_ptr,test);
				byte_offset = 0; // reset the byte offset
				block_offset ++; // increment the block offset by 1
				block_index = get_index_by_offset(file_inode, block_offset); // use the func to get the next block index.
				remaining_size -= BLOCK_SIZE; // reduce the remaining size by the amount of bytes that has been read.
			}
		}
		i ++;
	}
	return size_requested;
}

size_t f_write(void *restrict_ptr, size_t size, size_t nitems, int fd) {
	// Check if it is a valid file descriptor. Then, checking if the file is open or not.
	// Next, check if the user has the right permission to write the file.
	file_node *target_file = open_file_table[fd]; // get the file
	printf("This is f_write test printing! &&&&&&&&&&&&&&&&&&&&&&&&&\n");
	if (fd < 0 || fd > MAX_OPEN_FILE) {
		printf("Invalid file descriptor.\n");
		return FAIL;
	} else if (target_file->inode_entry == -1) {
		printf("This file has not been opened.\n");
		return FAIL;
	} else if (target_file->mode != WRONLY && target_file->mode != WRONLY + RDONLY && target_file->mode != WRONLY + RDONLY + EXEONLY) {
		printf("Permission denied.\n");
		return FAIL;
	} else if (size <= 0 || nitems <=0) {
		printf("Invalid size or nitems.\n");
		return FAIL;
	}

	inode* inode_of_file = disk_inode_region[target_file->inode_entry];
	long starting_size = (target_file->block_offset - 1) * BLOCK_SIZE;
	long data_block_index = starting_size / BLOCK_SIZE;
	void* rounded_buffer;
	void* transfer_buffer = rounded_buffer;
	size_t num_byte_need_to_copy = size * nitems;
	int curr_block = target_file->block_index;
	int curr_offset = target_file->byte_offset;
	long blocks_needed;
	blocks_needed = num_byte_need_to_copy + curr_offset;
	if (blocks_needed % BLOCK_SIZE == 0) {
		blocks_needed = blocks_needed / BLOCK_SIZE;
	}
	else {
		blocks_needed = blocks_needed / BLOCK_SIZE + 1;
	}
	int padding = starting_size + blocks_needed * BLOCK_SIZE - curr_offset - num_byte_need_to_copy;
	if (padding > 511) {
		printf("I died here! It sucks!!!\n");
		return FAIL;
	}
	// now our buffer looks like following:
	// origin data in the block ||| data we need to copy  ||| padding that fills the block
	rounded_buffer = malloc(blocks_needed * BLOCK_SIZE);
	bzero(rounded_buffer, blocks_needed * BLOCK_SIZE);
	lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + curr_block * BLOCK_SIZE, SEEK_SET);
	read(disk, rounded_buffer, curr_offset);
	transfer_buffer = (void*) ((char*) rounded_buffer + curr_offset);
	memcpy(transfer_buffer, restrict_ptr, num_byte_need_to_copy);
	long block_to_write = -1;
	for (int i = 0; i < blocks_needed; i++) {
		data_block_index = starting_size / BLOCK_SIZE;
		// cases when we rewrite current block
		if (i == 0 && curr_offset != 0) {
			block_to_write = curr_block;
			transfer_buffer = (void*) ((char*) rounded_buffer + i * BLOCK_SIZE);
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE, SEEK_SET);
			write(disk, transfer_buffer, BLOCK_SIZE);
			starting_size += BLOCK_SIZE;
			continue;
		}
		if (data_block_index < N_DBLOCKS) {
			int new_dblock = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			inode_of_file->dblocks[data_block_index] = new_dblock;
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + target_file->inode_entry * sizeof(inode), SEEK_SET);
			write(disk, inode_of_file, sizeof(inode));
			// write data to the new_dblock
			block_to_write = new_dblock;
			transfer_buffer = (void*) ((char*) rounded_buffer + i * BLOCK_SIZE);
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE, SEEK_SET);
			write(disk, transfer_buffer, BLOCK_SIZE);
			starting_size += BLOCK_SIZE;
		}
		else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS) {
			data_block_index -= N_DBLOCKS;
			int new_data_block;
			new_data_block = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			int i1block_index = (data_block_index) / NUM_INODE_IN_BLOCK;
			int i1block_offset = (data_block_index) % NUM_INODE_IN_BLOCK;
			if (i1block_offset == 0){
				int new_data_block_for_i1;
				new_data_block_for_i1 = sb->free_block;
				sb->free_block = get_next_free_block(sb->free_block);
				//i1block_index++; // we are now at the new i1block
				inode_of_file->iblocks[i1block_index] = new_data_block_for_i1;
				// write new_Data_block to the new i1block
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + inode_of_file->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[0] = new_data_block;
				// set the file indicator back to where it was, shit
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + inode_of_file->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}
			else {
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[i1block_offset] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}			
			block_to_write = new_data_block;
			transfer_buffer = (void*) ((char*) rounded_buffer + i * BLOCK_SIZE);
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE, SEEK_SET);
			write(disk, transfer_buffer, BLOCK_SIZE);
			starting_size += BLOCK_SIZE;
		}
		else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK) {
			data_block_index = data_block_index - N_DBLOCKS - NUM_INODE_IN_BLOCK * N_IBLOCKS;
			int new_data_block;
			new_data_block = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			int i2block_offset = (data_block_index) / NUM_INODE_IN_BLOCK;
			void *i2block_buffer = malloc(BLOCK_SIZE);
			// read data block of i2block
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + inode_of_file->i2block * BLOCK_SIZE, SEEK_SET);
			read(disk, i2block_buffer, BLOCK_SIZE);
			int *i2block_inode = (int *)i2block_buffer;
			// calculate index and offset in the given i1block
			int i1block_index = i2block_inode[i2block_offset];
			int i1block_offset = (data_block_index) % NUM_INODE_IN_BLOCK;
			if (i1block_offset == 0) {
				int new_data_block_for_i1;
				new_data_block_for_i1 = sb->free_block;
				sb->free_block = get_next_free_block(sb->free_block);
				i2block_inode[i2block_offset] = new_data_block_for_i1;
				// write new_Data_block to the new i1block
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[0] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + inode_of_file->i2block * BLOCK_SIZE, SEEK_SET);
				write(disk, i2block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}
			else {
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[i1block_offset] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);				
			}
			block_to_write = new_data_block;
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE, SEEK_SET);
			transfer_buffer = (void*) ((char*) rounded_buffer + i * BLOCK_SIZE);
			write(disk, transfer_buffer, BLOCK_SIZE);
			starting_size += BLOCK_SIZE;
			std::free(i2block_buffer);
		}
		else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK) {
			data_block_index = data_block_index - N_DBLOCKS - NUM_INODE_IN_BLOCK * N_IBLOCKS - NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK;
			int new_data_block;
			new_data_block = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);			
			// calculate which i2block we will step into
			int i3block_offset = (data_block_index) / (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK);
			// read the data block of i3block
			void *i3block_buffer = malloc(BLOCK_SIZE);
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + inode_of_file->i3block * BLOCK_SIZE, SEEK_SET);
			read(disk, i3block_buffer, BLOCK_SIZE);
			int *i3block_inode = (int *)i3block_buffer;

			// calculate which i1block we will step into
			int i2block_index = i3block_inode[i3block_offset];
			int i2block_offset = (data_block_index) % (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK);
			i2block_offset = i2block_offset / NUM_INODE_IN_BLOCK;

			// read the data block of i2block
			void *i2block_buffer = malloc(BLOCK_SIZE);
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i2block_index * BLOCK_SIZE, SEEK_SET);
			read(disk, i2block_buffer, BLOCK_SIZE);
			int *i2block_inode = (int *)i2block_buffer;

			// calculate which data block of i1block we will step into
			int i1block_index = i2block_inode[i2block_offset];
			int i1block_offset = (data_block_index) % NUM_INODE_IN_BLOCK;

			void *i1block_buffer = malloc(BLOCK_SIZE);
			// read data block from i1block
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
			read(disk, i1block_buffer, BLOCK_SIZE);
			int *inode_i1bloc = NULL;
			inode_i1bloc = (int *)(i1block_buffer);

			if (i1block_offset == 0) {
				int new_data_block_for_i1;
				new_data_block_for_i1 = sb->free_block;
				sb->free_block = get_next_free_block(sb->free_block);
				std::free(i1block_buffer);
				i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[0] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				// we need a new i2block
				// check whether i2block can hold one more i1block
				if ((data_block_index) % (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK) == 0){
					int new_data_block_for_i2;
					new_data_block_for_i2 = sb->free_block;
					sb->free_block = get_next_free_block(sb->free_block);
					i3block_inode[i3block_offset] = new_data_block_for_i2;
					std::free(i2block_buffer);
					i2block_buffer = malloc(BLOCK_SIZE);
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i2 * BLOCK_SIZE, SEEK_SET);
					read(disk, i2block_buffer, BLOCK_SIZE);
					int *inode_i2bloc = NULL;
					inode_i2bloc = (int *)(i2block_buffer);
					inode_i2bloc[0] = new_data_block_for_i1;
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i2 * BLOCK_SIZE, SEEK_SET);
					write(disk, i2block_buffer, BLOCK_SIZE);
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + inode_of_file->i3block * BLOCK_SIZE, SEEK_SET);
					write(disk, i3block_buffer, BLOCK_SIZE);
				}
				else {
					i2block_inode[i2block_offset] = new_data_block_for_i1;
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i2block_index * BLOCK_SIZE, SEEK_SET);
					write(disk, i2block_buffer, BLOCK_SIZE);
				}
			}
			else {
				std::free(i1block_buffer);
				i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[i1block_offset] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);				
			}
			block_to_write = new_data_block;
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE, SEEK_SET);
			transfer_buffer = (void*) ((char*) rounded_buffer + i * BLOCK_SIZE);
			write(disk, transfer_buffer, BLOCK_SIZE);
			starting_size += BLOCK_SIZE;
			std::free(i1block_buffer);
			std::free(i2block_buffer);
			std::free(i3block_buffer);
		}
		curr_block = block_to_write;
	}

	int return_value = starting_size - padding - (target_file->block_offset - 1) * BLOCK_SIZE + curr_offset;
	if ((target_file->block_offset - 1) * BLOCK_SIZE + num_byte_need_to_copy > inode_of_file->size) {
		inode_of_file->size = (target_file->block_offset - 1) * BLOCK_SIZE + num_byte_need_to_copy;
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + target_file->inode_entry * sizeof(inode), SEEK_SET);
		write(disk, inode_of_file, sizeof(inode));
	}
	if (padding == 0) {
		target_file->block_offset = starting_size / BLOCK_SIZE + 1;
	}
	else {
		target_file->block_offset = starting_size / BLOCK_SIZE;
	}
	target_file->block_index = curr_block;
	target_file->byte_offset = (BLOCK_SIZE - padding) % BLOCK_SIZE;

	return return_value;
}





// f_close needs to take of closing a normal file or a directory (maybe).
// Not tested yet. Maybe buggy!!!!!!!!
int f_close(int fd)
{
	// Check if it is a valid file descriptor. Then, checking if the file is open or not.
	file_node *target_file = open_file_table[fd]; // get the file
	if (fd > MAX_OPEN_FILE) {
		printf("Invalid file descriptor.\n");
		return FAIL;
	} else if (target_file->inode_entry == -1) {
		printf("This file has not been opened.\n");
		return FAIL;
	}
	int inode_index = target_file->inode_entry; // get the inode index
	inode* file_inode = disk_inode_region[inode_index]; // get the inode with the inode index
	// if the file is a directory file
	if (file_inode->type == DIRECTORY_FILE) {
		f_closedir(fd); // invoke f_closedir to close the directory
		return SUCCESS;
	}
	// if the file is a normal file
	target_file->inode_entry = -1; // change the inode entry to -1
	target_file->block_index = 0; // reset it back to 0.
	target_file->block_offset = 0;
	target_file->byte_offset = 0;
	target_file->mode = 0;
	open_file_table[fd] = target_file; // put the modified file node back into the open file table.
	//test printing
	printf("This is f_close test printing!**************************\n");
	print_file_status(fd);
	return SUCCESS;
}

// f_stat: get the information about the file given a file descriptor
// it seems to take care of both the normal file and the directory file. Not Tested yet. Maybe Buggy!!!!
int f_stat(int fd, struct fileStat *info)
{
	// Check if it is a valid file descriptor. Then, checking if the file is open or not.
	file_node *target_file = open_file_table[fd]; // get the file
	if (fd > MAX_OPEN_FILE) {
		printf("Invalid file descriptor.\n");
		return FAIL;
	} else if (target_file->inode_entry == -1) {
		printf("This file has not been opened.\n");
		return FAIL;
	}
	int inode_index = target_file->inode_entry;
	inode *target_file_inode = disk_inode_region[inode_index]; // get the corresponidng inode for the file
	// update the info
	info->inode_index = inode_index;
	info->filesize = target_file_inode->size;
	info->permission = target_file_inode->permission;
	info->uid = target_file_inode->uid;
	info->type = target_file_inode->type;
	info->gid = target_file_inode->gid;
	return SUCCESS;
}

int create_file(const string filename, int parent_inode, int type, int mode)
{
	if (num_of_open_file > MAX_OPEN_FILE)
	{
		return EXIT_FAILURE;
	}

	long parent_size = disk_inode_region[parent_inode]->size;
	long num_of_file_in_directory = parent_size / sizeof(directory_entry);
	long data_block_index = (parent_size - 1) / BLOCK_SIZE; // avoide cases when parent_size is 512*n bytes
	long data_block_offset = parent_size % BLOCK_SIZE;
	int cur_free_inode = sb->free_inode;
	int next_free_inode = disk_inode_region[cur_free_inode]->next_inode;
	int file_descriptor = -1;
	inode *parent = disk_inode_region[parent_inode];

	// update inode info in memory
	disk_inode_region[cur_free_inode]->nlink = 1;
	disk_inode_region[cur_free_inode]->permission = mode;
	disk_inode_region[cur_free_inode]->type = type;
	disk_inode_region[cur_free_inode]->parent = parent_inode;
	disk_inode_region[cur_free_inode]->size = 0;
	std::strcpy(disk_inode_region[cur_free_inode]->file_name, filename.c_str());

	// write inode back to disk
	lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + cur_free_inode * sizeof(inode), SEEK_SET);
	write(disk, disk_inode_region[cur_free_inode], sizeof(inode));

	// put the file into open file table
	for (int i = 0; i < MAX_OPEN_FILE; i++)
	{
		if (open_file_table[i]->inode_entry == -1)
		{
			open_file_table[i]->inode_entry = cur_free_inode;
		}
	}

	// update the data block of parent node
	directory_entry *to_update = (directory_entry *)malloc(sizeof(directory_entry));
	to_update->inode_entry = cur_free_inode;
	std::strcpy(to_update->file_name, filename.c_str());
	int block_to_write = -1;
	if (data_block_index < N_DBLOCKS && parent_size < BLOCK_SIZE * N_DBLOCKS)
	{
		// need new data block and update corresponding inode
		if (data_block_offset == 0) {
			int new_index;
			new_index = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			parent->dblocks[data_block_index + 1] = new_index;
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + parent_inode * sizeof(inode), SEEK_SET);
			write(disk, parent, sizeof(inode));
			block_to_write = new_index;
		}
		else {
			block_to_write = parent->dblocks[data_block_index];
		}
	}
	else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS && parent_size < BLOCK_SIZE * (N_DBLOCKS + N_IBLOCKS * NUM_INODE_IN_BLOCK))
	{
		data_block_index -= N_DBLOCKS;
		if (data_block_index < 0) {
			data_block_index = 0;
		}
		int i1block_index = (data_block_index) / NUM_INODE_IN_BLOCK;
		int i1block_offset = (data_block_index) % NUM_INODE_IN_BLOCK; // whether take 1 off here?
		// if we need a new data block
		if (data_block_offset == 0){
			int new_data_block;
			new_data_block = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			// if current i1block is filled up, we need to have a new data block for i1block
			if (i1block_offset == NUM_INODE_IN_BLOCK - 1){
				int new_data_block_for_i1;
				new_data_block_for_i1 = sb->free_block;
				sb->free_block = get_next_free_block(sb->free_block);
				i1block_index++; // we are now at the new i1block
				parent->iblocks[i1block_index] = new_data_block_for_i1;
				// write new_Data_block to the new i1block
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[0] = new_data_block;
				// set the file indicator back to where it was, shit
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}
			// update the i1block 
			else {
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[i1block_offset + 1] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}
			block_to_write = new_data_block;
		}
		else {
			void *i1block_buffer = malloc(BLOCK_SIZE);
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
			read(disk, i1block_buffer, BLOCK_SIZE);
			int *inode_i1bloc = NULL;
			inode_i1bloc = (int *)(i1block_buffer);
			block_to_write = inode_i1bloc[i1block_offset];
			std::free(i1block_buffer);
		}
	}
	else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK  && parent_size < BLOCK_SIZE * (N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK))
	{
		data_block_index = data_block_index - N_DBLOCKS - NUM_INODE_IN_BLOCK * N_IBLOCKS;
		if (data_block_index < 0) {
			data_block_index = 0;
		}
		// calculate which i1block to read (index in the data block of i2block)
		int i2block_offset = (data_block_index) / NUM_INODE_IN_BLOCK;
		void *i2block_buffer = malloc(BLOCK_SIZE);
		// read data block of i2block
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i2block * BLOCK_SIZE, SEEK_SET);
		read(disk, i2block_buffer, BLOCK_SIZE);
		int *i2block_inode = (int *)i2block_buffer;
		// calculate index and offset in the given i1block
		int i1block_index = i2block_inode[i2block_offset];
		int i1block_offset = (data_block_index) % NUM_INODE_IN_BLOCK;
		// we need a new data block
		if (data_block_offset == 0) {
			int new_data_block;
			new_data_block = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			if (i1block_offset == NUM_INODE_IN_BLOCK - 1){
				int new_data_block_for_i1;
				new_data_block_for_i1 = sb->free_block;
				sb->free_block = get_next_free_block(sb->free_block);
				i2block_offset++; // we are now at the new i1block
				i2block_inode[i2block_offset] = new_data_block_for_i1;
				// write new_Data_block to the new i1block
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[0] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i2block * BLOCK_SIZE, SEEK_SET);
				write(disk, i2block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}
			// update the i1block 
			else {
				void *i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[i1block_offset + 1] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
				std::free(i1block_buffer);
			}			
			block_to_write = new_data_block;
		}
		else {
			void *i1block_buffer = malloc(BLOCK_SIZE);
			// read data block from i1block
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
			read(disk, i1block_buffer, BLOCK_SIZE);
			int *inode_i1bloc = NULL;
			inode_i1bloc = (int *)(i1block_buffer);
			block_to_write = inode_i1bloc[i1block_offset];
			std::free(i1block_buffer);
		}
		std::free(i2block_buffer);
	}
	else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK)
	{
		data_block_index = data_block_index - N_DBLOCKS - NUM_INODE_IN_BLOCK * N_IBLOCKS - NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK;
		if (data_block_index < 0) {
			data_block_index = 0;
		}		
		// calculate which i2block we will step into
		int i3block_offset = (data_block_index) / (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK);
		// read the data block of i3block
		void *i3block_buffer = malloc(BLOCK_SIZE);
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i3block * BLOCK_SIZE, SEEK_SET);
		read(disk, i3block_buffer, BLOCK_SIZE);
		int *i3block_inode = (int *)i3block_buffer;

		// calculate which i1block we will step into
		int i2block_index = i3block_inode[i3block_offset];
		int i2block_offset = (data_block_index) % (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK);
		i2block_offset = i2block_offset / NUM_INODE_IN_BLOCK;

		// read the data block of i2block
		void *i2block_buffer = malloc(BLOCK_SIZE);
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i2block_index * BLOCK_SIZE, SEEK_SET);
		read(disk, i2block_buffer, BLOCK_SIZE);
		int *i2block_inode = (int *)i2block_buffer;

		// calculate which data block of i1block we will step into
		int i1block_index = i2block_inode[i2block_offset];
		int i1block_offset = (data_block_index) % NUM_INODE_IN_BLOCK;

		void *i1block_buffer = malloc(BLOCK_SIZE);
		// read data block from i1block
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
		read(disk, i1block_buffer, BLOCK_SIZE);
		int *inode_i1bloc = NULL;
		inode_i1bloc = (int *)(i1block_buffer);

		if (data_block_offset == 0){
			int new_data_block;
			new_data_block = sb->free_block;
			sb->free_block = get_next_free_block(sb->free_block);
			// we need a new i1block
			if (i1block_offset == NUM_INODE_IN_BLOCK - 1){
				int new_data_block_for_i1;
				new_data_block_for_i1 = sb->free_block;
				sb->free_block = get_next_free_block(sb->free_block);
				// we need a new i2block
				// check whether i2block can hold one more i1block
				std::free(i1block_buffer);
				i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[0] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i1 * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);

				if ((data_block_index) % (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK) == NUM_INODE_IN_BLOCK - 1){
					int new_data_block_for_i2;
					new_data_block_for_i2 = sb->free_block;
					sb->free_block = get_next_free_block(sb->free_block);
					/*code --------------------------------------------------------------------------------------------------------*/
					i3block_offset++;
					i3block_inode[i3block_offset] = new_data_block_for_i2;
					std::free(i2block_buffer);
					i2block_buffer = malloc(BLOCK_SIZE);
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i2 * BLOCK_SIZE, SEEK_SET);
					read(disk, i2block_buffer, BLOCK_SIZE);
					int *inode_i2bloc = NULL;
					inode_i2bloc = (int *)(i2block_buffer);
					inode_i2bloc[0] = new_data_block_for_i1;
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + new_data_block_for_i2 * BLOCK_SIZE, SEEK_SET);
					write(disk, i2block_buffer, BLOCK_SIZE);
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i3block * BLOCK_SIZE, SEEK_SET);
					write(disk, i3block_buffer, BLOCK_SIZE);					
				}
				else {
					i2block_offset++; // we are now at the new i1block
					i2block_inode[i2block_offset] = new_data_block_for_i1;
					lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i2block_index * BLOCK_SIZE, SEEK_SET);
					write(disk, i2block_buffer, BLOCK_SIZE);
				}
			}
			else {
				std::free(i1block_buffer);
				i1block_buffer = malloc(BLOCK_SIZE);
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				read(disk, i1block_buffer, BLOCK_SIZE);
				int *inode_i1bloc = NULL;
				inode_i1bloc = (int *)(i1block_buffer);
				inode_i1bloc[i1block_offset + 1] = new_data_block;
				lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
				write(disk, i1block_buffer, BLOCK_SIZE);
			}
			block_to_write = new_data_block;
		}
		else {
			lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
			read(disk, i1block_buffer, BLOCK_SIZE);
			int *inode_i1bloc = NULL;
			inode_i1bloc = (int *)(i1block_buffer);
			block_to_write = inode_i1bloc[i1block_offset];
		}

		std::free(i3block_buffer);
		std::free(i2block_buffer);
		std::free(i1block_buffer);
	}
	else
	{
		return EXIT_FAILURE;
	}

	// write new directory entry to given position in the data block
	lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE + data_block_offset, SEEK_SET);
	write(disk, to_update, sizeof(directory_entry));
	// update parent file size
	parent->size += sizeof(directory_entry);
	lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + parent_inode * sizeof(inode), SEEK_SET);
	write(disk, parent, sizeof(inode));
	std::free(to_update);
	return cur_free_inode;
}


int get_next_free_block(int block_index) {
	void* data_buffer = malloc(BLOCK_SIZE);
	lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_index * BLOCK_SIZE, SEEK_SET);
	read(disk, data_buffer, BLOCK_SIZE);
	int* data = (int*) data_buffer;
	int result = data[0];
	free(data_buffer);
	return result;
}

void print_file_status(int fd) {
	  printf("the %d th element, inode_entry is %d\n",fd,open_file_table[fd]->inode_entry);
	  printf("the %d th element, block_index is %d\n",fd,open_file_table[fd]->block_index);
	  printf("the %d th element, block_offset is %d\n",fd,open_file_table[fd]->block_offset);
	  printf("the %d th element, byte_offset is %d\n",fd,open_file_table[fd]->byte_offset);
	  printf("the %d th element, mode is %d\n",fd,open_file_table[fd]->mode);
}
void print_file_table() {
	if(open_file_table[0] == NULL) {
		printf("not initialized yet\n");
		return;
	}
	for(int i = 0; i < MAX_OPEN_FILE; i++) {
		print_file_status(i);
	}
}
