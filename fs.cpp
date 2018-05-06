#include "fs.h"

Superblock *sb; // super block
file_node open_file_table[MAX_OPEN_FILE];
inode disk_inode_region[MAX_INODE_NUM];
int cur_directory;
int num_of_total_data_block;
int num_of_total_inode;
int disk;

using namespace std;

// update root in superblock, update inode, open root directory
/*
int f_mount(char* destination, char* diskname) {
	int fd;
	fd = open(diskname, O_RDWR);
	if (fd == -1) {
		fd = format_default_size(diskname);
		if (fd == -1) {
			return EXIT_FAILURE;
		}
		else {
			open(fd, O_RDWR);
		}
	}
	sb->root = destination;
	long int filesize = 0;
	fseek(fd, 0, SEEK_END);
	filesize = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);
	
	for (int i = 0; i < num_of_total_inode; ++i) {
		disk_inode_region[i] = malloc(sizeof(inode));
		fseek(fd, BOOT_SIZE + SUPER_SIZE + i * sizeof(inode), SEEK_SET);
		read(fd, disk_inode_region[i], sizeof(inode));
	}
	disk_inode_region[0]->nlink = 1;
	disk_inode_region[0]->filename = destination;
	fseek(fd, BOOT_SIZE + SUPER_SIZE, SEEK_SET);
	write(fd, disk_inode_region[0], sizeof(inode));
	for (int i = 0; i < MAX_OPEN_FILE; i++) {
		open_file_table[i] = malloc(sizeof(file_node));
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
}
*/

int format_default_size(string filename)
{
	int fd;
	//in open we need to convert to c-style string
	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT);
	if (fd == -1)
	{
		return EXIT_FAILURE;
	}
	ftruncate(fd, DEFAULT_SIZE);
	char *bootblock = (char *)malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	int out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE)
	{
		return EXIT_FAILURE;
	}
	free(bootblock);
	sb = (Superblock *)malloc(sizeof(Superblock));
	sb->size = BLOCK_SIZE;
	sb->inode_offset = 0;
	sb->data_offset = DEFAULT_SIZE * INODE_RATE / BLOCK_SIZE;
	sb->free_inode = 0;
	sb->free_block = 0;

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
		default_inode->dblocks[i] = 0;
	}

	for (int i = 0; i < N_IBLOCKS; i++)
	{
		default_inode->iblocks[i] = 0;
	}

	default_inode->i2block = 0;
	default_inode->i3block = 0;
	//can't directly assign, use strcpy
	//default_inode->file_name = "";
	strcpy(default_inode->file_name, "");
	//default_inode->padding = 0;

	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i)
	{
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

	free(default_inode);
	close(fd);
	//need to replace by predefined
	return 0;
}

int format_with_given_size(string filename, long int file_size)
{
	int fd;
	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT);
	if (fd == -1)
	{
		return EXIT_FAILURE;
	}
	ftruncate(fd, file_size);
	char *bootblock = (char *)malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	int out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE)
	{
		return EXIT_FAILURE;
	}
	free(bootblock);
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
	strcpy(default_inode->file_name, "");
	//default_inode->padding = 0;

	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i)
	{
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

	free(default_inode);
	close(fd);
	disk = fd; //why we need to store it?
	//need to replace by predefined
	return 0;
}

// we need to parse the command to see which format we will use

/*
int f_seek(int fd, long int offset, char* whence) {
	if (fd < 0 or fd > MAX_OPEN_FILE) {
		return EXIT_FAILURE;
	}
	file_node* cur_file = open_file_table[fd];
	if (cur_file == NULL) {
		// the file has not been opened
		return EXIT_FAILURE;
	}
	int inode_idx = cur_file->inode_entry;
	if (inode_idx > num_of_total_inode) {
		// the file doesn't exist
		return EXIT_FAILURE;
	}
	else if (disk_inode_region[inode_idx]->nlink < 1) {
		return EXIT_FAILURE;
	}
	else {
		if (strcmp(whence, "SEEK_SET") == 0) {
			if (offset > disk_inode_region[inode_idx]->size) {
				return EXIT_FAILURE;
			}
			cur_file->block_offset = offset / BLOCK_SIZE;
			cur_file->byte_offset = offset % BLOCK_SIZE;
		}
		else if (strcmp(whence, "SEEK_CUR") == 0) {
			long int to_modify = cur_file->block_offset * BLOCK_SIZE + cur_file->byte_offset + offset;
			if (to_modify > disk_inode_region[inode_idx]->size) {
				return EXIT_FAILURE;
			}
			cur_file->block_offset = to_modify / BLOCK_SIZE;
			cur_file->byte_offset = to_modify % BLOCK_SIZE;
		}
		else if (strcmp(whence, "SEEK_END") == 0) {
			if (offset > 0) {
				return EXIT_FAILURE;
			}
			long int to_modify = disk_inode_region[inode_idx]->size - offset;
			cur_file->block_offset = to_modify / BLOCK_SIZE;
			cur_file->byte_offset = to_modify % BLOCK_SIZE;
		}
	}
	return EXIT_SUCCESS;
}


void rewind(int fd) {
	if (fd < 0 or fd > MAX_OPEN_FILE) {
		return EXIT_FAILURE;
	}
	file_node* cur_file = open_file_table[fd];
	if (cur_file == NULL) {
		// the file has not been opened
		return EXIT_FAILURE;
	}
	int inode_idx = cur_file->inode_entry;
	if (inode_idx > num_of_total_inode) {
		// the file doesn't exist
		return EXIT_FAILURE;
	}
	else if (disk_inode_region[inode_idx]->nlink < 1) {
		return EXIT_FAILURE;
	}
	else {
		cur_file->block_offset = 0;
		cur_file->byte_offset = 0;
	}
	return EXIT_SUCCESS;
}
*/

//dir functions

struct dirent *f_opendir(char *path)
{
	//we can directly use f_open to do this
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

int add_to_file_table(int inode_num, inode f_node)
{
	//get the inode from inode region
	int i;
	for (i = 0; i < MAX_OPEN_FILE; i++)
	{
		if (open_file_table[i].inode_entry == 0)
		{
			open_file_table[i].inode_entry = inode_num;
			open_file_table[i].block_index = f_node.dblocks[0];
			open_file_table[i].block_offset = 0;
			open_file_table[i].byte_offset = 0;
			break;
		}
	}
	return i;
}

// disk_buffer needs to be changed... I need the disk image to be in a buffer to get its data region.
// use read and lseek
// Maybe buggy...... Needs to be tested.
int traverse_dir(int dirinode_index, string filename)
{
	//bool IsExist = false;
	//size_t size_of_disk = 6666666666;				  // IT MUST BE CHANGED LATER.
	//char *disk_buffer = (char *)malloc(size_of_disk); // IMPORTANT: hard coded buffer that contains the disk image data!!!, This needs to be changed later.
	// first, check if the given inode has an entry in the disk inode region (i.e., Check if the directory exists or not)
	inode direct = disk_inode_region[dirinode_index]; // get the directory
	if (direct.nlink == 0)
	{
		printf("This directory does not exist.\n");
		return FAIL;
	}
	// then, check if the given inode is a directory inode or not
	if (direct.type != DIRECTORY_FILE)
	{
		printf("This is not a directory.\n");
		return FAIL;
	}
	// now lets check the data region to check if the given filename exists or not
	// first, look into the direct data blocks
	size_t remaining_size = direct.size; // get the size of the directory, and says it is the remaining size.
	int data_offset = sb->data_offset;   // get the data offset from the super block
	size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
	for (int i = 0; i < N_DBLOCKS; i++)
	{
		int data_block_index = direct.dblocks[i];
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
			else if (remaining_size <= 0)
			{
				return FAIL;
			}
			remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
		}
	}
	// if the remaining_size is not 0, we still need to look into other data blocks.
	// Then, look into the single indirect data blocks
	size_t ptr_addr;
	for (int i = 0; i < N_IBLOCKS; i++)
	{
		ptr_addr = data_region_starting_addr + BLOCK_SIZE * direct.iblocks[i];
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
				else if (remaining_size <= 0)
				{
					return FAIL;
				}
				remaining_size -= sizeof(directory_entry);
			}
		}
	}
	// Next, look into the double indirect data blocks
	size_t i2block_ptr_addr = data_region_starting_addr + direct.i2block * BLOCK_SIZE;
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
				else if (remaining_size <= 0)
				{
					return FAIL;
				}
				remaining_size -= sizeof(directory_entry);
			}
		}
	}

	// Finally, look into the triple indirect data blocks
	size_t i3block_ptr_addr = data_region_starting_addr + direct.i3block * BLOCK_SIZE;
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
					else if (remaining_size <= 0)
					{
						return FAIL;
					}
					remaining_size -= sizeof(directory_entry);
				}
			}
		}
	}
	// if it still doesn't return, return a -1.
	return FAIL;
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
	for (int i = 0; i < path_list.size(); i++)
	{
		dir_node = traverse_dir(dir_node, path_list[i]); //and need to consider permission problem
		//not that simple need to consider restrict_mode
		if (dir_node == -1)
		{
			cout << "The file path is incorrect" << endl;
			return FAIL;
		}
	}
	//if we get out of loop, it should get the target file's inode then we set up the open file table
	/*set up open file table：
		1.find the index of this file in open file table -- return value
		2.find inode index
		3.find the first data block index
		4.set the rest
	*/
	inode target = disk_inode_region[dir_node];
	int result = add_to_file_table(dir_node, target);
	//we also need to deal with open_file_table, have a function to add and remove element in open file table
	return result;
}

int create_file (const string filename, int parent_inode, int type) {
	if (num_of_open_file > MAX_OPEN_FILE) {
		return EXIT_FAILURE;
	}

	unsigned long parent_size = disk_inode_region[parent_inode]->size;
	unsigned long num_of_file_in_directory = parent_size / sizeof(directory_entry);
	unsigned long data_block_index = (parent_size - 1) / BLOCK_SIZE; // avoide cases when parent_size is 512*n bytes
	unsigned long data_block_offset = parent_size % BLOCK_SIZE;
	int cur_free_inode = sb->free_inode;
	int next_free_inode = disk_inode_region[cur_free_inode]->next_inode;
	int file_descriptor = -1;
	inode* parent = disk_inode_region[parent_inode];

	// update inode info in memory
	disk_inode_region[cur_free_inode]->nlink = 1;
	disk_inode_region[cur_free_inode]->permission = 7;
	disk_inode_region[cur_free_inode]->type = type;
	disk_inode_region[cur_free_inode]->parent = parent_inode;
	disk_inode_region[cur_free_inode]->size = 0;
	strcpy(disk_inode_region[cur_free_inode]->file_name, filename);

	// write inode back to disk
	lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + cur_free_inode * sizeof(inode), SEEK_SET);
	write(disk, disk_inode_region[cur_free_inode], sizeof(inode));

	// put the file into open file table
	for (int i = 0; i < MAX_OPEN_FILE; i++) {
		if (open_file_table[i] == -1) {
			open_file_table[i]->inode_entry = cur_free_inode;
		}
	}

	// update the data block of parent node
	directory_entry* to_update = (directory_entry*) malloc(sizeof(directory_entry));
	to_update->inode_entry = cur_free_inode;
	strcpy(to_update->inode_entry, filename);
	int block_to_write = -1;
	if (data_block_index < N_DBLOCKS) {
		block_to_write = parent->dblocks[data_block_index];
	}
	else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS) {
		data_block_index -= N_DBLOCKS;
		int i1block_index = (data_block_index - 1) / NUM_INODE_IN_BLOCK;
		int i1block_offset = (data_block_index - 1) % NUM_INODE_IN_BLOCK;
		void* i1block_buffer = malloc(BLOCK_SIZE);
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->iblocks[i1block_index] * BLOCK_SIZE, SEEK_SET);
		fread(i1block_buffer, 1, BLOCK_SIZE, disk);
		int* inode_i1bloc = NULL;
		inode_i1bloc = (int*) (i1block_buffer);
		block_to_write = inode_i1bloc[i1block_offset];
		free(i1block_buffer);
	}
	else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK) {
		data_block_index = data_block_index - N_DBLOCKS - NUM_INODE_IN_BLOCK * N_IBLOCKS;
		// calculate which i1block to read (index in the data block of i2block)
		int i2block_offset = (data_block_index - 1) / NUM_INODE_IN_BLOCK;
		void* i2block_buffer = malloc(BLOCK_SIZE);
		// read data block of i2block
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i2block * BLOCK_SIZE, SEEK_SET);
		fread(i2block_buffer, 1, BLOCK_SIZE, disk);
		int* i2block_inode = (int*) i2block_buffer;
		// calculate index and offset in the given i1block
		int i1block_index = i2block_inode[i2block_offset];
		int i1block_offset = (data_block_index - 1) % NUM_INODE_IN_BLOCK;
		
		void* i1block_buffer = malloc(BLOCK_SIZE);
		// read data block from i1block
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
		fread(i1block_buffer, 1, BLOCK_SIZE, disk);
		int* inode_i1bloc = NULL;
		inode_i1bloc = (int*) (i1block_buffer);
		block_to_write = inode_i1bloc[i1block_offset];

		free(i2block_buffer);
		free(i1block_buffer);
	}
	else if (data_block_index < N_DBLOCKS + NUM_INODE_IN_BLOCK * N_IBLOCKS + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK + NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK) {
		data_block_index = data_block_index - N_DBLOCKS - NUM_INODE_IN_BLOCK * N_IBLOCKS - NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK;
		// calculate which i2block we will step into
		int i3block_offset = (data_block_index - 1) / (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK);
		// read the data block of i3block
		void* i3block_buffer = malloc(BLOCK_SIZE);
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i3block * BLOCK_SIZE, SEEK_SET);
		fread(i3block_buffer, 1, BLOCK_SIZE, disk);
		int* i3block_inode = (int*) i3block_buffer;

		// calculate which i1block we will step into
		int i2block_index = i3block_inode[i3block_offset];
		int i2block_offset = (data_block_index - 1) % (NUM_INODE_IN_BLOCK * NUM_INODE_IN_BLOCK);
		i2block_offset = i2block_offset / NUM_INODE_IN_BLOCK;

		// read the data block of i2block
		void* i2block_buffer = malloc(BLOCK_SIZE);
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + parent->i2block_index * BLOCK_SIZE, SEEK_SET);
		fread(i2block_buffer, 1, BLOCK_SIZE, disk);
		int* i2block_inode = (int*) i2block_buffer;

		// calculate which data block of i1block we will step into
		int i1block_index = i2block_inode[i2block_offset];
		int i1block_offset = (data_block_index - 1) % NUM_INODE_IN_BLOCK;
		
		void* i1block_buffer = malloc(BLOCK_SIZE);
		// read data block from i1block
		lseek(disk, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i1block_index * BLOCK_SIZE, SEEK_SET);
		fread(i1block_buffer, 1, BLOCK_SIZE, disk);
		int* inode_i1bloc = NULL;
		inode_i1bloc = (int*) (i1block_buffer);
		block_to_write = inode_i1bloc[i1block_offset];

		free(i3block_buffer);
		free(i2block_buffer);
		free(i1block_buffer);		
	}
	else {
		return EXIT_FAILURE;
	}

	// write new directory entry to given position in the data block
	lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + block_to_write * BLOCK_SIZE + data_block_offset, SEEK_SET);
	write(fd, to_update, sizeof(directory_entry));
	free(to_update);


}

