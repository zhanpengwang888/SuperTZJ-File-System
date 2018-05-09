#ifndef TEST_UTIL_H
#define TEST_UTIL_H

using namespace std;
#include "../file_util.h"
#define TRUE 1
#define FALSE 0
//testing variable lists
extern FILE* fp;
extern char* file_buffer;
extern Superblock *sp; // super block
extern int inode_start;
extern int inode_end;
extern int inode_num;
//file_node *open_file_table[MAX_OPEN_FILE];
extern inode *disk_inode_list[MAX_INODE_NUM];
extern std::string junk;
extern const char* junk_c;
void update_sp();
//buffer should be the buffer stores the whole disk
void print_inode_region(Superblock* sp, char* buffer);
void print_inode(inode* Inode, int index, int detail);
inode* next_inode(inode* Inode);
void print_superblock(Superblock* sp);
void print_directory(Superblock* sp, inode* dir_node, char* buffer);
void create_mid_node(inode* test_inode, int index, int size, string name);
void create_test_file(char* name);
void create_node(inode* test_inode, int index, int size, string name);
void create_mid_file(char* name);
void create_large_file(char* name);
void print_free_blocks(char* buffer, Superblock* sp);
#endif
