#ifndef FS_H
#define FS_H
#include "file_util.h"

int f_mount(char* destination, char* diskname);
int f_unmount(char* diskname);
int format_default_size(char* filename);
int format_with_given_size(char* filename, long int file_size);
int f_seek(int fd, long int offset, char* whence);
int f_open(const char *restrict_path, const char *restrict_mode);
size_t f_read(void *restrict_ptr, size_t size, size_t nitems, int fd);
size_t f_write(void *restrict_ptr, size_t size, size_t nitems, int fd);
int f_close(int fd);
int f_stat(int fd, struct stat *info);
int f_remove(const char *path);
struct dirent *f_opendir(char* path);
struct dirent *f_readdir(int dirfd);
int f_closedir(int dirfd);
int f_mkdir(char* path, int mode);
int f_rmdir(char* filepath);
void rewind(int fd);

#endif











































































