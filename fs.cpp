#include "fs.h"

Superblock* sb; // super block
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
		open_file_table[i]->inode_entry = 0;
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

int format_default_size(string filename){
	int fd;
	//in open we need to convert to c-style string
	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT);
	if (fd == -1) {
		return EXIT_FAILURE;
	}
	ftruncate(fd, DEFAULT_SIZE);
	char *bootblock = (char*)malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	int out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE) {
		return EXIT_FAILURE;
	}
	free(bootblock);
	sb = (Superblock*) malloc(sizeof(Superblock));
	sb->size = BLOCK_SIZE;
	sb->inode_offset = 0;
	sb->data_offset = DEFAULT_SIZE * INODE_RATE / BLOCK_SIZE;
	sb->free_inode = 0;
	sb->free_block = 0;

	num_of_total_data_block = (DEFAULT_SIZE - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
	//instead of fseek, we should use lseek for file descriptor
	lseek(fd, BOOT_SIZE, SEEK_SET);
	write(fd, sb, SUPER_SIZE);

	for (int i = 0; i < num_of_total_data_block; i++) {
		if (i == num_of_total_data_block - 1) {
			int to_write = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &to_write, sizeof(int));
		}
		else {
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &i, sizeof(int));
		}
	}

	// construct a default inode
	inode* default_inode = (inode*) malloc(sizeof(inode));
	default_inode->nlink = 0;
	default_inode->permission = 7;
	default_inode->type = NORMAL_FILE;
	default_inode->next_inode = 1;
	default_inode->size = 0;
	default_inode->uid = 0;
	default_inode->gid = 0;
	for (int i = 0; i < N_DBLOCKS; i++) {
		default_inode->dblocks[i] = 0;
	}

	for (int i = 0; i < N_IBLOCKS; i++) {
		default_inode->iblocks[i] = 0;
	}

	default_inode->i2block = 0;
	default_inode->i3block = 0;
	//can't directly assign, use strcpy
	//default_inode->file_name = "";
	strcpy(default_inode->file_name,"");
	default_inode->padding = 0;

	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i) {
		if (i == num_of_total_inode - 1) {
			default_inode->next_inode = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
		else {
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


int format_with_given_size(string filename, long int file_size){
	int fd;
	fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT);
	if (fd == -1) {
		return EXIT_FAILURE;
	}
	ftruncate(fd, file_size);
	char *bootblock = (char*) malloc(BOOT_SIZE);
	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
	int out = write(fd, bootblock, BOOT_SIZE);
	if (out != BOOT_SIZE) {
		return EXIT_FAILURE;
	}
	free(bootblock);
	sb = (Superblock*) malloc(sizeof(Superblock));
	sb->size = BLOCK_SIZE;
	sb->inode_offset = 0;
	sb->data_offset = file_size * INODE_RATE / BLOCK_SIZE;
	sb->free_inode = 0;
	sb->free_block = 0;

	num_of_total_data_block = (file_size - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
	lseek(fd, BOOT_SIZE, SEEK_SET);
	write(fd, sb, SUPER_SIZE);

	for (int i = 0; i < num_of_total_data_block; i++) {
		if (i == num_of_total_data_block - 1) {
			int to_write = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &to_write, sizeof(int));
		}
		else {
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
			write(fd, &i, sizeof(int));
		}
	}

	// construct a default inode
	inode* default_inode = (inode*) malloc(sizeof(inode));
	default_inode->nlink = 0;
	default_inode->permission = 7;
	default_inode->type = NORMAL_FILE;
	default_inode->next_inode = 1;
	default_inode->size = 0;
	default_inode->uid = 0;
	default_inode->gid = 0;
	for (int i = 0; i < N_DBLOCKS; i++) {
		default_inode->dblocks[i] = 0;
	}

	for (int i = 0; i < N_IBLOCKS; i++) {
		default_inode->iblocks[i] = 0;
	}

	default_inode->i2block = 0;
	default_inode->i3block = 0;
	//default_inode->file_name = "";
	strcpy(default_inode->file_name,"");
	default_inode->padding = 0;

	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

	for (int i = 0; i < num_of_total_inode; ++i) {
		if (i == num_of_total_inode - 1) {
			default_inode->next_inode = -1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
		else {
			default_inode->next_inode = i + 1;
			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
			write(fd, default_inode, sizeof(inode));
		}
	}

	free(default_inode);
	close(fd);
	disk = fd;
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

struct dirent *f_opendir(char* path) {
	//we can directly use f_open to do this

}

int f_open(const string restrict_path, const string restrict_mode) {
	//we need to check mode here to avoid invalid mode
	//how can we go into the disk to find the 
	//parse the path to know the filename
	//we can copy the inode region to memory! so search the inode region to see whether the file is in it
	//if the file is in the disk...  we set up the open file table 
	/*set up open file table：
		1.find the index of this file in open file table -- return value
		2.find inode index
		3.find the first data block index
		4.set the rest
}























































