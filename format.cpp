#include "fs.h"
#include "Testing/test_util.h"


/*
Our disk image:

----------------------------------------------------
			Boot block: 512 bytes
----------------------------------------------------
			Super block: 512 bytes
----------------------------------------------------
			Inode region
----------------------------------------------------
			Data block
----------------------------------------------------
*/


// int format_default_size(char* filename){
// 	int fd;
// 	fd = open(filename, O_RDWR | O_TRUNC | O_CREAT);
// 	if (fd == -1) {
// 		return EXIT_FAILURE;
// 	}
// 	ftruncate(fd, DEFAULT_SIZE);
// 	char *bootblock = (char*)malloc(BOOT_SIZE);
// 	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
// 	int out = write(fd, bootblock, BOOT_SIZE);
// 	if (out != BOOT_SIZE) {
// 		return EXIT_FAILURE;
// 	}
// 	free(bootblock);
// 	sb = (Superblock*) malloc(sizeof(Superblock));
// 	sb->size = BLOCK_SIZE;
// 	sb->inode_offset = 0;
// 	sb->data_offset = DEFAULT_SIZE * INODE_RATE / BLOCK_SIZE;
// 	sb->free_inode = 0;
// 	sb->free_block = 0;

// 	num_of_total_data_block = (DEFAULT_SIZE - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
// 	//instead of fseek, we should use lseek for file descriptor
// 	lseek(fd, BOOT_SIZE, SEEK_SET);
// 	write(fd, sb, SUPER_SIZE);

// 	for (int i = 0; i < num_of_total_data_block; i++) {
// 		if (i == num_of_total_data_block - 1) {
// 			int to_write = -1;
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
// 			write(fd, &to_write, sizeof(int));
// 		}
// 		else {
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
// 			write(fd, &i, sizeof(int));
// 		}
// 	}

// 	// construct a default inode
// 	inode* default_inode = (inode*) malloc(sizeof(inode));
// 	default_inode->nlink = 0;
// 	default_inode->permission = 7;
// 	default_inode->type = NORMAL_FILE;
// 	default_inode->next_inode = 1;
// 	default_inode->size = 0;
// 	default_inode->uid = 0;
// 	default_inode->gid = 0;
// 	for (int i = 0; i < N_DBLOCKS; i++) {
// 		default_inode->dblocks[i] = 0;
// 	}

// 	for (int i = 0; i < N_IBLOCKS; i++) {
// 		default_inode->iblocks[i] = 0;
// 	}

// 	default_inode->i2block = 0;
// 	default_inode->i3block = 0;
// 	//can't directly assign, use strcpy
// 	//default_inode->file_name = "";
// 	strcpy(default_inode->file_name,"");
// 	default_inode->padding = 0;

// 	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

// 	for (int i = 0; i < num_of_total_inode; ++i) {
// 		if (i == num_of_total_inode - 1) {
// 			default_inode->next_inode = -1;
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
// 			write(fd, default_inode, sizeof(inode));
// 		}
// 		else {
// 			default_inode->next_inode = i + 1;
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
// 			write(fd, default_inode, sizeof(inode));
// 		}
// 	}

// 	free(default_inode);
// 	close(fd);
// 	//need to replace by predefined
// 	return 0;
// }


// int format_with_given_size(char* filename, long int file_size){
// 	int fd;
// 	fd = open(filename, O_RDWR | O_TRUNC | O_CREAT);
// 	if (fd == -1) {
// 		return EXIT_FAILURE;
// 	}
// 	ftruncate(fd, file_size);
// 	char *bootblock = (char*) malloc(BOOT_SIZE);
// 	memset(bootblock, DEFAULT_DATA, BOOT_SIZE);
// 	int out = write(fd, bootblock, BOOT_SIZE);
// 	if (out != BOOT_SIZE) {
// 		return EXIT_FAILURE;
// 	}
// 	free(bootblock);
// 	sb = (Superblock*) malloc(sizeof(Superblock));
// 	sb->size = BLOCK_SIZE;
// 	sb->inode_offset = 0;
// 	sb->data_offset = file_size * INODE_RATE / BLOCK_SIZE;
// 	sb->free_inode = 0;
// 	sb->free_block = 0;

// 	num_of_total_data_block = (file_size - BOOT_SIZE - SUPER_SIZE - (sb->data_offset - sb->inode_offset) * BOOT_SIZE) / BOOT_SIZE;
// 	lseek(fd, BOOT_SIZE, SEEK_SET);
// 	write(fd, sb, SUPER_SIZE);

// 	for (int i = 0; i < num_of_total_data_block; i++) {
// 		if (i == num_of_total_data_block - 1) {
// 			int to_write = -1;
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
// 			write(fd, &to_write, sizeof(int));
// 		}
// 		else {
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->data_offset * BLOCK_SIZE + i * BLOCK_SIZE, SEEK_SET);
// 			write(fd, &i, sizeof(int));
// 		}
// 	}

// 	// construct a default inode
// 	inode* default_inode = (inode*) malloc(sizeof(inode));
// 	default_inode->nlink = 0;
// 	default_inode->permission = 7;
// 	default_inode->type = NORMAL_FILE;
// 	default_inode->next_inode = 1;
// 	default_inode->size = 0;
// 	default_inode->uid = 0;
// 	default_inode->gid = 0;
// 	for (int i = 0; i < N_DBLOCKS; i++) {
// 		default_inode->dblocks[i] = 0;
// 	}

// 	for (int i = 0; i < N_IBLOCKS; i++) {
// 		default_inode->iblocks[i] = 0;
// 	}

// 	default_inode->i2block = 0;
// 	default_inode->i3block = 0;
// 	//default_inode->file_name = "";
// 	strcpy(default_inode->file_name,"");
// 	default_inode->padding = 0;

// 	num_of_total_inode = (sb->data_offset - sb->inode_offset) * BOOT_SIZE / sizeof(inode);

// 	for (int i = 0; i < num_of_total_inode; ++i) {
// 		if (i == num_of_total_inode - 1) {
// 			default_inode->next_inode = -1;
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
// 			write(fd, default_inode, sizeof(inode));
// 		}
// 		else {
// 			default_inode->next_inode = i + 1;
// 			lseek(fd, BOOT_SIZE + SUPER_SIZE + sb->inode_offset * BLOCK_SIZE + i * sizeof(inode), SEEK_SET);
// 			write(fd, default_inode, sizeof(inode));
// 		}
// 	}

// 	free(default_inode);
// 	close(fd);
// 	disk = fd;
// 	//need to replace by predefined
// 	return 0;
// }

// we need to parse the command to see which format we will use


//this program should call those two format functions in the fs.cpp and create correspond disk image
int main (int args, char** argv) {
	if (args != 2 && args != 4) {
		printf("Please input the name of disk you want to format!\n");
		printf("Or if you want to DIY your disk, add -s # (size of disk in number of bytes) after name of disk.\n");
		return EXIT_FAILURE;
	}
	char* name = argv[1];
	printf("the name of disk is %s\n",name);
	if(args == 2) {
		format_default_size(string(name));
	}
	else {
		size_t size = atoi(argv[3]);
		if(size < 0) {
			printf("Please input a number in last argument\n");
			return EXIT_FAILURE;
		}
		format_with_given_size(string(name),size);
	}
	return SUCCESS;
}










