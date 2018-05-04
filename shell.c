/*
structure inspired by: https://github.com/jmreyes/simple-c-shell/blob/master/simple-c-shell.c
			https://github.com/stpddream/OSHomework/tree/master/hw3
*/
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <pwd.h>
#include "parser.h"
#include "run_command.h"
#include "ll.h"

#define MAXLEN 256
#define MAXLINE 1024
#define NEGATIVE -1

// function prototypes.
void ourPrompt();
void initShell();
void sigchld_handler(int sig, siginfo_t *sif, void *notused);

// global variables
char **args_without_pipe; //  xxx | xxx | xxx | ...
char **args;			  // arguments in each *command
char **command;			  // command line that has been separated by ";"
char *line;				  // the whole command line
char *jobLine;			  // things we will store in job struct
sigset_t child_mask;

void sigchld_handler(int sig, siginfo_t *sif, void *notused)
{
	// first get the process id of the child process and get the pgid of the child process
	pid_t pgid = sif->si_pid;
	if (pgid == NEGATIVE)
	{
		perror("Get pgid error:");
		exit(EXIT_FAILURE);
	}
	// Then get the job with pgid;
	Job *job = getJobPid(pgid);
	if (job == NULL)
	{
		return;
	}
	//check what is the exit status of the calling process
	/* if the job normally exit, update its status, print the job status, remove it from job list
			 and then free the memory. Else if the job is suspended, update its status and print its status.
			 Else if the job is forced to terminate, update its status, print its status, remove it from the
			 job list, and then free the memory.
		*/
	if (sif->si_code == CLD_EXITED)
	{
		jobChangeStatus(job, JOBCOMP);
		printJobStatus(job);
		return;
	}
	if (sif->si_code == CLD_STOPPED)
	{
		jobChangeStatus(job, JOBSTOP);
		last_suspended = job->jobId;
		printJobStatus(job);
		return;
	}
	if (sif->si_code == CLD_KILLED)
	{
		jobChangeStatus(job, JOBTERM);
		printJobStatus(job);
		return;
	}
	return;
}

// print our shell prompt
void ourPrompt()
{
	struct passwd *pwd; // using struct passwd from pwd.h
	char host[MAXLEN];
	char path[MAXLINE];
	pwd = getpwuid(getuid());		   // get information about the user.
	getcwd(path, MAXLINE);			   // get the current working directory
	char *username = pwd->pw_name;	 // username
	char *homedirectory = pwd->pw_dir; // home directory
									   // a little bit verbose, restore when all bug fixed
	if (gethostname(host, MAXLEN) == 0)
	{
		printf("[用心写bug，用脚写shell, (ヘ･_･)ヘ┳━┳ ┫ ┻━┻ ︵ヽ(`Д´)ﾉ︵ ┻━┻ ┣ﾍ(^▽^ﾍ)Ξ(ﾟ▽ﾟ*)ﾉ┳━┳, ¯\\_(ツ)_/¯ TZJ Shell]\n[%s@%s ", username, host);
	}
	else
		printf("[用心写bug，用脚写shell, (ヘ･_･)ヘ┳━┳ ┫ ┻━┻ ︵ヽ(`Д´)ﾉ︵ ┻━┻ ┣ﾍ(^▽^ﾍ)Ξ(ﾟ▽ﾟ*)ﾉ┳━┳, ¯\\_(ツ)_/¯ TZJ Shell]\n[%s@unknown ", username);

	if (strlen(path) < strlen(homedirectory) || strncmp(path, homedirectory, strlen(homedirectory)))
		printf("%s]", path);
	else
		printf("~%s]", path);

	// root: /  guest/user: $
	if (geteuid() == 0)
		printf("/");
	else
		printf("$");
	return;
}

void initShell()
{
	myShTerminal = STDOUT_FILENO;
	if (!isatty(myShTerminal))
		perror("not a tty device");
	else
	{
		if (tcgetattr(myShTerminal, &myShTmodes) != 0)
			perror("tcgetattr error");
		if ((myShPGid = tcgetpgrp(myShTerminal)) < 0)
		{
			perror("tcgetpgrp() failed");
		}
	}
}

int main(int argc, char **argv)
{
	jobInit();
	initShell();
	int built_in_flag = FALSE;
	check_stat_pid = -10;
	/* Sigchild signal handling*/
	//declare the sample sigaction struct
	struct sigaction sa;
	//set the handler, in this case our handler has three arguments, so will set sa_sigaction instead of sa_handler.
	sa.sa_sigaction = &sigchld_handler;
	//initialize an empty set for signals to be blocked.
	sigemptyset(&sa.sa_mask);
	//set flags, SA_SIGINFO flag is to specify signal handler in sa is sa_sigaction.
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	//register the signal SIGCHLD.
	sigaction(SIGCHLD, &sa, NULL);

	//ignore most signals in parent shell
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	sigemptyset(&child_mask);
	sigaddset(&child_mask, SIGINT);
	sigaddset(&child_mask, SIGTSTP);
	sigaddset(&child_mask, SIGTERM);
	sigaddset(&child_mask, SIGQUIT);
	sigaddset(&child_mask, SIGTTOU);
	sigaddset(&child_mask, SIGTTIN);
	sigaddset(&child_mask, SIGCHLD);

	int numCommands;
	int numArguments;
	//int numSegments;
	while (1)
	{
		// print our shell prompt
		ourPrompt();
		command = (char **)malloc(MAXLINE * sizeof(char *));
		bzero(command, MAXLINE);
		if ((line = readline(" ")) == NULL)
		{
			free(command);
			freeJobList();
			perror("Exit the shell.\nLog out successfully.");
			exit(EXIT_SUCCESS);
		}
		numCommands = parseCommands(line, command); // parse a command line into different
													// processes (by ";")
		if (numCommands < 0)
		{
			freeArgs(command);
			printf("Syntax error\n");
			free(line);
			continue;
		}

		for (int i = 0; i < numCommands; i++)
		{
			jobLine = (char *)malloc(sizeof(char) * strlen(command[i]) + 1);
			bzero(jobLine, strlen(command[i]) + 1);
			args_without_pipe = (char **)malloc(sizeof(char *) * MAXLEN);
			strcpy(jobLine, command[i]);
			bzero(args_without_pipe, MAXLEN);
			numArguments = parseArguments(command[i], args_without_pipe);

			Process *toAdd = (Process *)malloc(sizeof(Process));
			toAdd->args = args_without_pipe;
			toAdd->argn = numArguments;
			toAdd->next = NULL;

			if (args_without_pipe != NULL && args_without_pipe[0] != NULL)
			{
				if (strcmp(args_without_pipe[0], "exit") == 0)
				{
					freeProcess(toAdd);
					free(jobLine);
					freeJobList();
					free(line);
					freeArgs(command);
					return 0;
				}
			}
			if (args_without_pipe != NULL && args_without_pipe[0] != NULL)
			{
				int ret;
				if (strcmp(args_without_pipe[0], "cd") == 0 && args_without_pipe[1] == NULL)
				{
					const char *home = getenv("HOME");
					ret = chdir(home);
					if (ret == -1)
						perror("cd");
				}
				else if (strcmp(args_without_pipe[0], "cd") == 0 && args_without_pipe[1] != NULL)
				{
					ret = chdir(args_without_pipe[1]);
					if (ret == -1)
						perror("cd");
				}
			}
			int field;
			int status;
			if (numArguments < 0)
			{
				field = JOBBACK;
				toAdd->argn = -numArguments;
			}
			else if (numArguments > 0)
			{
				field = JOBFORE;
			}
			else
			{
				free(jobLine);
				freeProcess(toAdd);
				break;
			}
			status = JOBRUN;
			Job *job = createJob(jobLine, toAdd, status, field);
			free(jobLine);

			//free(dummy);// shouldn't free it here, process will be automatically freed when the job is removed from joblist
			if (!check_built_in(job) && jobInsert(job) == FALSE)
			{
				printf("Add job fail!\n");
				exit(1);
			}
			if (check_built_in(job))
			{
				id--;
				built_in_flag = TRUE;
			}

			executing_command_without_pipe(job, child_mask);

			if (built_in_flag)
			{
				freeJob(job);
				built_in_flag = FALSE;
			}

			Node *temp = head->next;
			while (temp != NULL && temp->job != NULL)
			{
				Node *next = temp->next;
				if (temp->job->field == JOBBACK || temp->job->field == JOBFORE)
				{
					if (temp->job->status == JOBCOMP || temp->job->status == JOBTERM)
					{
						jobs_lock(child_mask);
						jobRemovePid(temp->job->pgid);
						jobs_unlock(child_mask);
					}
				}
				temp = next;
			}
		}
		free(line);
		freeArgs(command);
	}
	return 0;
}
