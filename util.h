#ifndef UTIL_
#define UTIL_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>

#define DEFAULT_NUM_PROCESS 1024
#define DEFAULT_NUM_SEGMENTS 1024
#define DEFAULT_NUM_ARG 1024
#define FAIL -1
enum {FALSE, TRUE}; // FALSE 0, TRUE 1;
enum {JOBCOMP, JOBSTOP, JOBRUN, JOBBACK, JOBFORE, JOBTERM}; // JOBCOMP 0, JOBSTOP 1, JOBRUN 2, JOBBACK 3, JOBFORE 4, JOBTERM 5
enum {PROCCOMP, PROCSTOP, PROCRUN}; // PROCCOMP 0, PROCSTOP 1, PROCRUN 2
//globally used myshell status variable
int myShTerminal;
pid_t myShPGid;
struct termios myShTmodes;
pid_t check_stat_pid;

#endif
