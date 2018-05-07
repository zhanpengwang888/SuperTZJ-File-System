#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "../file_util.h"
#define TRUE 1
#define FALSE 0
//buffer should be the buffer stores the whole disk
void print_inode_region(Superblock* sp, char* buffer);
void print_inode(inode* Inode, int index, int detail);
inode* next_inode(inode* Inode);
void print_superblock(Superblock* sp);
void print_directory(Superblock* sp, inode* dir_node, char* buffer);

#endif
