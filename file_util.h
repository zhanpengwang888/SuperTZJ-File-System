#ifndef FILE_UTIL_H
#define	FILE_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//added Sat 12pm -- flag problem
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>

#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define BLOCK_SIZE 512
#define SUPER_SIZE 512
#define BOOT_SIZE 512
#define MAX_OPEN_FILE 175
#define INODE_RATE 1/100
#define DEFAULT_DATA   'a'
#define DEFAULT_SIZE 1048576  // 1MB
#define DIRECTORY_FILE 0
#define NORMAL_FILE 1
#define MAX_INODE_NUM 100000
#define INODE_SIZE sizeof(inode)


typedef struct superblock {
	int size;
	int inode_offset;
	int data_offset;
	int free_inode;
	int free_block;
	int root;
} Superblock;

// the sizeof inode is 128 byte, and thus a block can hold 4 inode entries.
typedef struct Inode {
	int nlink;
	int permission;
	int type;
	int parent;
	int next_inode;
	int size;
	int uid;
	int gid;
	int dblocks[N_DBLOCKS];
	int iblocks[N_IBLOCKS];
	int i2block;
	int i3block;
	char file_name[28];
	int padding;
} inode;
//entry in open file table
typedef struct file_entry {
	int inode_entry;
	int block_offset;
	int byte_offset;
} file_node;

// the whole directory_entry struct is thus 4 + 28 = 32 bytes, and in other words, a block can hold 16 directory entries.
typedef struct DirectoryEntry {
	int inode_entry;
	char file_name[28];
} directory_entry;

extern Superblock* sb; // super block
extern file_node open_file_table[MAX_OPEN_FILE];
extern inode disk_inode_region[MAX_INODE_NUM];
extern int cur_directory;
extern int num_of_total_data_block;
extern int num_of_total_inode;
extern int disk;
#endif





	
























 

























































