#ifndef FS_H
#define FS_H
#include "file_util.h"
using namespace std;
//I change all the char* to string, keep those char* used as pointer instead of string
int f_mount(char* destination, string diskname);
int f_unmount(string diskname);
int format_default_size(string filename);
int format_with_given_size(string filename, long int file_size);
int f_seek(int fd, long int offset, char* whence);
int f_open(const string restrict_path, const string restrict_mode);
size_t f_read(void *restrict_ptr, size_t size, size_t nitems, int fd);
size_t f_write(void *restrict_ptr, size_t size, size_t nitems, int fd);
int f_close(int fd);
int f_stat(int fd, struct fileStat *info);
int f_remove(const string path);
struct dirent *f_opendir(string path); // I think the argument should not be a path, if so, how to open root directory?
struct dirent *f_readdir(int dirfd);
int f_closedir(int dirfd);
int f_mkdir(string path, int mode);
int f_rmdir(string filepath);
void rewind(int fd);
int traverse_dir(int dirinode_index, string filename);
int get_next_free_block(int block_index);
#endif











































































