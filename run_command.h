#ifndef RUNCOMM_
#define RUNCOMM_

#include <signal.h>
#include <string.h>
#include <termios.h>
#include "ll.h"
#include "util.h"
#include "fs.h"

void bKill(char** args, int argn);
void bFg(char** args, int argn, sigset_t child_mask);
void bBg(char** args, int argn);
void bJobs();
void executing_command_without_pipe(Job* job, sigset_t child_mask);
void jobs_lock(sigset_t child_mask);
void jobs_unlock(sigset_t child_mask);
int exeBuiltIn(char** args, int argn, sigset_t child_mask);
int changeDirectory(char** args);
int check_built_in(Job* job);
void put_job_in_foreground(Job* job, sigset_t child_mask, int flag_stop);
void ls(int mode);
int fs_cd(char **args, int argn);
void fs_pwd();
void fs_chmod(char **args, int argn);
int fs_mkdir(char** args, int argn);
int fs_rmdir(char** args, int argn);
#endif
