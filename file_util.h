#ifndef FILE_UTIL_H
#define	FILE_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define BLOCK_SIZE 512
#define MAX_OPEN_FILE 175


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



	

































#endif

























































