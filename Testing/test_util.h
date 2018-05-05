#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <iostream>
#include "../file_util.h"
#define TRUE 1
#define FALSE 0

void print_inode(inode* inode, int index, int detail);
inode* next_inode(inode* inode);
void print_superblock(Superblock* sp);
#endif