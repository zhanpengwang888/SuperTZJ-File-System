#ifndef FILE_UTIL_H
#define	FILE_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <errno.h>
//added Sat 12pm -- flag problem
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
//c++ lib
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

//for function return value
#define FAIL -1
#define SUCCESS 0

//super block information
#define BLOCK_SIZE 512
#define SUPER_SIZE 512
#define BOOT_SIZE 512
#define MAX_OPEN_FILE 175 //why 175?

#define DEFAULT_DATA   'a' //sha?
#define DEFAULT_SIZE 1048576  // 1MB
//for file type
#define DIRECTORY_FILE 0
#define NORMAL_FILE 1
//inode information
#define MAX_INODE_NUM 1000
#define INODE_SIZE sizeof(inode)
#define INODE_RATE 1/100
#define N_DBLOCKS 10
#define N_IBLOCKS 4
#define NUM_INODE_IN_BLOCK (BLOCK_SIZE / sizeof(int))
#define EXEONLY 1
#define WRONLY 2
#define RDONLY 4

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
	int type; // tell you it is a file or directory
	int parent; //
	int next_inode;
	unsigned long size;
	int uid;
	int gid;
	int dblocks[N_DBLOCKS];
	int iblocks[N_IBLOCKS];
	int i2block;
	int i3block;
	char file_name[28];
	//int padding;
} inode;
//entry in open file table
typedef struct file_entry {
	int inode_entry;
	int block_index; // this is the data block we will go and retrieve data from. if we want to find the next block, we need to use block_offset to know the index of next data block
	int block_offset; //the index of block in this file, not the whole disk
	int byte_offset;
	int mode;
} file_node;

// the whole directory_entry struct is thus 4 + 28 = 32 bytes, and in other words, a block can hold 16 directory entries.
typedef struct DirectoryEntry {
	int inode_entry;
	char file_name[28];
} directory_entry;

// file stat struct for f_stat
struct fileStat {
	int uid;
	int gid;
	int filesize;
	int type; // dir or regular file
	int permission;
	int inode_index;
};

extern Superblock* sb; // super block
extern file_node* open_file_table[MAX_OPEN_FILE];
extern inode* disk_inode_region[MAX_INODE_NUM];
extern int cur_directory;
extern int num_of_total_data_block;
extern int num_of_total_inode;
extern int num_of_open_file;
extern int disk;
#endif





	
























 

























































