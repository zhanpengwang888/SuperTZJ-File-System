#include "run_command.h"
#define MAX_LEN 256
//need to implement joblock() and jobunlock() to protect joblist
//void jobunlock();
//void joblock();

int last_backgrounded; // keep track of the last backgrounded job
// built-in command: jobs
int fs_rm(char* file_name) {
	//parse the file_name list

	vector<string> f_list = split(string(file_name)," ");
	char* f_name = (char*)malloc(MAX_LEN);
	int result;
	for (int i = 0; i < f_list.size(); i++)
	{
		cout << "This element is " << f_list[i] << endl;
		//open the file to get is type
		strcpy(f_name,curr_path);
		strcat(f_name,"/");
		strcat(f_name,f_list[i].c_str());
		int test_fd = f_opendir(f_name)
		if(test_fd >= 0) {
			printf("Can remove directory use \"rm\" command, please use rmdir\n");
			return FAIL;
		}
		result = f_remove(f_name);
		if(result < 0) {
			printf("Something wrong with the f_remove\n");
			return FAIL
		}
	}
	return SUCCESS;
}

char* get_cur_file_path(char* cur_dir, char* cur_filename) {
        char* cur_file_path = (char*)malloc(256);
        strcpy(cur_file_path,cur_dir);
        strcat(cur_file_path,"/");
        strcat(cur_file_path,cur_filename);
        printf("cur_file is now %s\n",cur_file_path);
        return cur_file_path;
}

//ls need to know the current directory path
//readdir will return NULL if it is the end of directory
void fs_ls(char **args, int argn) {
	//mode 1 is -F
	//mode 2 is -l
	int mode;
	if (argn != 2) {
		return FAIL;
	}
	if (strcmp(args[1], "-F")) {
		mode = 1;
	}
	else if (strcmp(args[1], "-l")) {
		mode = 2;
	}
	else {
		return FAIL;
	}
	int cur_dir = f_opendir(curr_path);
	int cur_fd;
	struct fileStat* f_status = (struct fileStat *)malloc(sizeof(fileStat));
	if( cur_dir == -1) {
		printf("there is something wrong with cur_dir, it is %s\n",curr_path);
		//initalize cur_path and global_path
	}
	directory_entry* cur_entry;
	while(1){
		cur_entry = f_readdir(cur_dir);
		if(cur_entry == NULL) {
			//let the directory go back to the beginning
			//f_seek(cur_dir,0,"SEEK_SET");
			f_closedir(cur_dir);
			return;
		}
		if(mode == 1) {
			if(strcmp(cur_entry->file_name,".")== 0 || strcmp(cur_entry->file_name,"..") == 0) {
				printf("%s\t",cur_entry->file_name);
				continue;
			}
			char* cur_file = get_cur_file_path(curr_path,cur_entry->file_name);
			cur_fd = f_open(string(cur_file),"r");
			if(f_stat(cur_fd,f_status) < 0) {
				printf("there are some problems reading this file\n");
				continue;
			}
			if(f_status->type == DIRECTORY_FILE)
				printf("%s%s\t",cur_entry->file_name,"/");
			else if(f_status->permission == EXEONLY)
				printf("%s%s\t",cur_entry->file_name,"*");
			else
				printf("%s\t",cur_entry->file_name);
			free(cur_file);
		}
		else if(mode == 2) {
			if(strcmp(cur_entry->file_name,".")== 0 || strcmp(cur_entry->file_name,"..") == 0) {
				continue;
			}
			char* cur_file = get_cur_file_path(curr_path,cur_entry->file_name);
			cur_fd = f_open(string(cur_file),"r");
			if(f_stat(cur_fd,f_status) < 0) {
				printf("there are some problems reading this file\n");
				continue;
			}
			char* p_char = (char*)(malloc(4));
			switch(f_status->permission) {
				case 1:
					strcpy(p_char,"x--");
					break;
				case 2:
					strcpy(p_char,"--w");
					break;
				case 3:
					strcpy(p_char,"x-w");
					break;
				case 4:
					strcpy(p_char,"-r-");
					break;
				case 5:
					strcpy(p_char,"xr-");
					break;
				case 7:
					strcpy(p_char,"xrw");
					break;	
			}
			printf("%s   %d   %d    %d    %s\n",p_char,f_status->uid,f_status->gid,f_status->filesize, cur_entry->file_name);
			free(cur_file);
			free(p_char);
		}

		//deal with deafult mode
		else {
			if(strcmp(cur_entry->file_name,".")== 0 || strcmp(cur_entry->file_name,"..") == 0) {
				continue;
			}
			printf("%s\t",cur_entry->file_name);
		}
	}

}

void bJobs()
{
	printList();
	return;
}

void bKill(char **args, int argn)
{
	int status;
	int kill_flag = FALSE; //kill_flag is true when input has -9
	int dash_flag = 0;
	Job *job;
	int to_be_killed = 0;
	if (argn == 1)
	{
		printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
		return;
	}

	//parse the arguments and send proper respond
	for (int i = 1; i < argn; i++)
	{
		//if the input is invalid, we will print usage of kill and return
		if (dash_flag > 1)
		{
			printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
			return;
		}
		if (args[i][0] == '-')
		{
			dash_flag++;
			if (strcmp(args[i], "-9") == 0)
			{
				if (i != 1)
				{
					printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
					return;
				}
				kill_flag = TRUE; //probably we need to test whether kill_flag is false
			}
			else
			{
				printf("kill: usage: kill (signal) %%jid (or pid).Currently, signal only support -9, SIGKILL.\n");
				return;
			}
		}
		else if (atoi(args[i]) != 0)
		{
			job = getJobPid(atoi(args[i]));
			if (job == NULL)
			{
				job = getJobJobId(atoi(args[i]));
			}
			if (job == NULL)
			{
				printf("invalid job number or process number\n");
				return;
			}
			to_be_killed = (-1) * job->pgid;
			if (kill_flag)
			{
				if (kill(to_be_killed, SIGKILL) == -1)
				{
					perror("Kill failed\n");
				}
			}
			else
			{
				if (job->status == JOBSTOP)
				{
					kill(to_be_killed, SIGCONT);
					if (kill(to_be_killed, SIGTERM) == -1)
					{
						perror("Kill failed\n");
					}
				}
				else
				{
					if (kill(to_be_killed, SIGTERM) == -1)
					{
						perror("Kill failed\n");
					}
				}
			}
		}
		//if the element is %number
		else if (args[i][0] == '%' && atoi(args[i] + 1) != 0)
		{
			job = getJobPid(atoi(args[i] + 1));
			if (job == NULL)
			{
				job = getJobJobId(atoi(args[i] + 1));
			}
			if (job == NULL)
			{
				printf("invalid job number or process number\n");
				return;
			}
			to_be_killed = (-1) * job->pgid;
			if (kill_flag)
			{
				if (kill(to_be_killed, SIGKILL) == -1)
					perror("Kill failed\n");
			}
			else
			{
				if (job->status == JOBSTOP)
				{
					kill(to_be_killed, SIGCONT);
					if (kill(to_be_killed, SIGTERM) == -1)
					{
						perror("Kill failed\n");
					}
				}
				else
				{
					if (kill(to_be_killed, SIGTERM) == -1)
					{
						perror("Kill failed\n");
					}
				}
			}
		}

		//if the element is % or other symbol--ignore
		else
		{
			continue;
		}
		if (dash_flag > 0 && i > 1)
		{
			//wait until the sigkill sent
			waitpid(job->pgid, &status, 0);
		}
	}
}

void put_job_in_foreground(Job *job, sigset_t child_mask, int flag_stop)
{
	int status;
	//put the job into foreground;
	//shell_terminal is a global file descriptor represented as shell
	tcsetpgrp(myShTerminal, job->pgid);

	//stroe the current shell status to its termios
	//tcgetattr(myShTerminal, &myShTmodes);

	//restore the terminal attributes when the job stopped last time -- if the job used to be stopped, we restroe, otherwise, we ignore
	if (flag_stop)
		tcsetattr(myShTerminal, TCSADRAIN, &job->j_Tmodes);

	//we have already changed the status of process
	//wait foreground job to exit
	while (job->status != JOBCOMP && job->status != JOBSTOP && job->status != JOBTERM)
	{
		waitpid(job->pgid, &status, WUNTRACED); // since it's in foreground, we shouldn't use WNOHANG
	}

	//if the job complete, we exit the job
	if (job->status == JOBSTOP)
	{
		//if job has been suspended, we store its termios
		tcgetattr(myShTerminal, &job->j_Tmodes);
	}
	else
	{
		jobs_lock(child_mask); //need to implement, block all the possible access to job list
		jobRemoveJobId(job->jobId);
		jobs_unlock(child_mask);
	}

	/* Put the shell back in the foreground.  */
	tcsetpgrp(myShTerminal, myShPGid);

	/* Restore the shell’s terminal modes.  */
	tcsetattr(myShTerminal, TCSADRAIN, &myShTmodes);
}

void bFg(char **args, int argn, sigset_t child_mask)
{
	Job *current_job = NULL;
	int is_suspended = 0;

	if (argn == 1)
	{
		current_job = getJLastBackgrounded();
	}
	else if (argn == 2)
	{
		if (args[1][0] == '%' && atoi(args[1] + 1) != 0)
		{
			current_job = getJobJobId(atoi(args[1] + 1));
		}
		else if (atoi(args[1]) != 0)
		{
			current_job = getJobJobId(atoi(args[1]));
		}
		else
		{
			printf("Pleas type in %%[number]\n");
		}
	}
	else if (argn == 3)
	{
		if ((strcmp(args[1], "%") == 0) && atoi(args[2]) != 0)
		{
			current_job = getJobJobId(atoi(args[2]));
		}
		else
		{
			printf("Pleas type in %%[number]\n");
		}
	}
	else
	{
		printf("Pleas type in %%[number]\n");
	}

	//update job status
	//first check whether the job used to be stopped
	if (current_job == NULL)
	{
		printf("fg: No such job exist!\n");
		return;
	}
	if (current_job->status == JOBSTOP)
		is_suspended = 1;
	current_job->field = JOBFORE; //-- How do I know it is from stopped job or newly created job? how to keep track
	current_job->status = JOBRUN;
	pid_t current_pgid = -1 * current_job->pgid;
	if (kill(current_pgid, SIGCONT) < 0) {
		perror("kill (SIGCONT)"); // send sigcont to all processes of that process group
		return;
	}
	//we can directly send a signal to a process group

	//should add a flag to determine whether job used to be stopped
	put_job_in_foreground(current_job, child_mask, is_suspended);
}
// void bFg(char** args, int argn, sigset_t child_mask) {
// 	int jid; // get job id by args or use default setting
// 	Job* current_job = NULL;
// 	int is_suspended = 0;
// 	/*if(argn == 1)
// 		jid = num_of_job(); // implemented by jiaping, get the number of job in a job list
// 	else
// 		jid = atoi(args[1]); //convert the input job id to int

// 	current_job = getJobJobId(jid);
// 	if(current_job == NULL) {
// 		printf("job %d does not exist, we can find it\n", jid);
// 		return;
// 	}*/

// 	if (argn == 1) {
// 		current_job = getJLastBackgrounded();
// 	}
// 	else if (argn == 2){
// 		if (args[1][0] == '%' && atoi(args[1]+1) != 0) {
// 			current_job = getJobJobId(atoi(args[1]+1));
// 		}
// 		else {
// 			printf("Pleas type in %%[number]\n");
// 		}
// 	}
// 	else if(argn == 3) {
// 		if ((strcmp(args[1],"%") == 0) && atoi(args[2]) != 0) {
// 			current_job = getJobJobId(atoi(args[2]));
// 		}
// 		else {
// 			printf("Pleas type in %%[number]\n");
// 		}
// 	}
// 	else {
// 		printf("Pleas type in %%[number]\n");
// 	}

// 	//update job status
// 	//first check whether the job used to be stopped
// 	if(current_job->field == JOBSTOP)
// 		is_suspended = 1;
// 	current_job->field = JOBFORE; //-- How do I know it is from stopped job or newly created job? how to keep track
// 	current_job->status = JOBRUN;
// 	pid_t current_pgid = -1 * current_job->pgid;
// 	if(kill(current_pgid, SIGCONT) < 0)
// 		perror("kill (SIGCONT)"); // send sigcont to all processes of that process group
// 	//we can directly send a signal to a process group
// 	/*
// 	while(current_job_process != NULL) {
// 		//need to implement kill()
// 		kill(current_job_process->pid, SIGCONT)； // send sigcont to each process in job
// 		current_job_process->status = PROCRUN; // set process to run status
// 		current_job_process = current_job_process->next;
// 	}
// 	*/

// 	//should add a flag to determine whether job used to be stopped
// 	put_job_in_foreground(current_job, child_mask, is_suspended);

// }

// void bBg(char** args, int argn) {
// 	Job* current_job = NULL;
// 	if (argn == 1) {
// 		current_job = getJLastSuspended();
// 	}
// 	for(int i = 0; i < argn - 1; i++) {
// 		if (args[i][0] == '%' && atoi(args[i]+1) != 0) {
// 			current_job = getJobJobId(atoi(args[1]+1));
// 		}
// 		else if (atoi(args[i]) != 0) {

// 		}
// 	}
// }

// void bBg(char** args, int argn) {
// 	Job* current_job = NULL;
// 	if (argn == 1) {
// 		current_job = getJLastSuspended();
// 	}
// 	for(int i = 0; i < argn - 1; i++) {
// 		if (args[i][0] == '%' && atoi(args[i]+1) != 0) {
// 			current_job = getJobJobId(atoi(args[1]+1));
// 		}
// 		else if (atoi(args[i]) != 0) {
// 			current_job = getJobJobId(atoi(args[1]));
// 		}
// 		else {
// 			continue;
// 		}
// 		if (current_job == NULL) {
// 			printf("job does not exist, we can find it\n");
// 			continue;
// 		}
// 		else {
// 			current_job->field = JOBBACK;
// 			current_job->status = JOBRUN;
// 			last_backgrounded = current_job->jobId;

// 		}

// 	}
// }


int fs_cd(char **args, int argn) {
	if (argn != 2) {
		return FAIL;
	}
	else if (strcmp(args[0], "cd") != 0) {
		return FAIL;
	}
	int fd = -1;
	char *tocheck = (char *) malloc(strlen(args[1]) + strlen(curr_path) + 1);
	bzero(tocheck, strlen(args[1]) + strlen(curr_path) + 1);
	// check whether is an absolute path
	if (args[1][0] != '/') {
		free(tocheck);
		tocheck = NULL;
		fd = f_opendir(string(args[1]));
		if (fd < 0) {
			return FAIL;
		}
		else {
			f_closedir(fd);
			bzero(curr_path, 256);
			strcpy(curr_path, args[1]);
			return SUCCESS;
		}
	}
	else {
		strcpy(tocheck, curr_path);
		strcat(tocheck, args[1]);
		fd = f_opendir(string(args[1]));
		if (fd < 0) {
			free(tocheck);
			return FAIL;
		}
		else {
			f_closedir(fd);
			bzero(curr_path, MAX_LEN);
			strcpy(curr_path, tocheck);
			free(tocheck);
			return FAIL;
		}
	}
}

void fs_pwd() {
	char* temp = (char*)malloc(256);
	strcpy(temp, curr_path);
	free(curr_path);
	curr_path = pwd(string(curr_path));
	free(temp);
	printf("%s\n", curr_path);
}

int fs_chmod(char **args, int argn) {
	if (argn != 3) {
		return FAIL;
	}
	else if (atoi(args[1]) == 0 && strcmp(args[1], "0") != 0) {
		return FAIL;
	}
	else if (strcmp(args[0], "chmod") != 0) {
		return FAIL;
	}
	return change_mode(atoi(args[1]), string(args[2]));
}


int fs_mkdir(char** args, int argn) {
	if (argn != 3) {
		return FAIL;
	}
	else if (atoi(args[1]) == 0 && strcmp(args[1], "0") != 0) {
		return FAIL;
	}
	else {
		return f_mkdir(string(args[2]), atoi(args[1]));
	}
}

int fs_rmdir(char** args, int argn) {
	if (argn != 2) {
		return FAIL;
	}
	else {
		return f_rmdir(string(args[1]));
	}
}


int fs_mount(char** args, int argn) {
	if (argn != 2) {
		return FAIL;
	}
	else {
		return f_mount("/", args[1]); 
	}
}

int fs_unmount(char** args, int argn) {
	if (argn != 2) {
		return FAIL;
	}
	else {
		return f_unmount("/", args[1]); 
	}
}



// could be "bg %        number" or "bg %number"
void bBg(char **args, int argn)
{
	Job *current_job = NULL;
	if (argn == 1)
	{
		current_job = getJLastSuspended();
	}
	else if (argn == 2)
	{
		if (args[1][0] == '%' && atoi(args[1] + 1) != 0)
		{
			current_job = getJobJobId(atoi(args[1] + 1));
		}
		else if (atoi(args[1]) != 0)
		{
			current_job = getJobJobId(atoi(args[1]));
		}
		else
		{
			printf("Pleas type in %%[number]\n");
		}
	}
	else if (argn == 3)
	{
		if ((strcmp(args[1], "%") == 0) && atoi(args[2]) != 0)
		{
			current_job = getJobJobId(atoi(args[2]));
		}
		else
		{
			printf("Pleas type in %%[number]\n");
		}
	}
	else
	{
		printf("Pleas type in %%[number]\n");
	}

	if (current_job == NULL)
	{
		printf("job does not exist, we can find it\n");
		return;
	}

	//update job status
	current_job->field = JOBBACK;
	current_job->status = JOBRUN;
	last_backgrounded = current_job->jobId;
	pid_t current_pgid = -1 * current_job->pgid;
	if (kill(current_pgid, SIGCONT) < 0)
		perror("kill (SIGCONT)");
}

// cat built-in command implementation starts here
// Not Tested yet. Potential buggy.

string conversion(char **arg, int size) {
	string temp = "";
	for (int i = 1; i < size; i++) {
		temp += *arg[i];
	}
	return temp;
}

// get the keyboard input from stdin and write it into a file.
void microcat_stdin(int file_descriptor_to_be_written)
{
  int wr;
  char buf[1];
  while (read(0, buf, sizeof buf) > 0)
  {
    wr = f_write(buf, sizeof(buf), 1, file_descriptor_to_be_written);
	//write(file_descriptor_to_be_written, buf, sizeof buf);
    if (wr < 0)
    {
		printf("[microcat_stdin] Fails\n");
		break;
    }
  }
}

// get the keyboard input from stdin and write it into a file.
// syscall version
void microcat_stdin_using_syscall(int file_descriptor_to_be_written)
{
  int wr;
  char buf[1];
  while (read(0, buf, sizeof buf) > 0)
  {
    wr = write(file_descriptor_to_be_written, buf, sizeof buf);
    if (wr < 0)
    {
		printf("[microcat_stdin] Fails\n");
		break;
    }
  }
}

// check if there is a redirection '>' or ">>" or '<' in the give arguments
// check if the command only has > redirection.
int only_right_redirection(char **arg, int size)
{
  for (int i = 1; i < size; i++)
  {
    if (*arg[i] == '<')
      return FALSE;
  }
  return TRUE;
}

// check if it is the right redirection ">"
int IsRightRedirection(char **arg, int size)
{
	if(only_right_redirection(arg, size))
		return FALSE;
	string command = string(conversion(arg, size));
	vector<string> command_string_list = split(command, '>'); // split it by '>'
	vector<string> command_string_list_double = split(command, '>>'); // split by ">>"
	if (command_string_list.size() > 1 && command_string_list_double.size() <= 1)
		return TRUE;
	return FALSE;
}

// check if the command only has < redirection.
int only_left_redirection(char **arg, int size)
{
  for (int i = 1; i < size; i++)
  {
    if (*arg[i] == '>')
      return FALSE;
  }
  return TRUE;
}

// check if it is the left redirection "<"
int IsLeftRedirection(char **arg, int size)
{
	if(only_left_redirection(arg, size))
		return FALSE;
	string command = string(conversion(arg, size));
	vector<string> command_string_list = split(command, '<'); // split it by '>'
	if (command_string_list.size() > 1)
		return TRUE;
	return FALSE;
}

// check if the command is double redirection
int doubleRedirection(char **arg, int size) {
	string command = string(conversion(arg, size));
	vector<string> command_string_list = split(command, '>>'); // split it by ">>"
	if (command_string_list.size() > 1)
		return TRUE;
	return FALSE;
}

// our fs version
void microcat(const char *file_name, int file_descriptor_to_be_written)
{
  int fd, wr, rd;
  fd = f_open(string(file_name), "r");
  //(file_name, O_RDONLY);

  int size_of_file = get_file_size(string(file_name));
  char buffer[size_of_file + 1];

  // error checking and handling
  if (fd < 0)
  {
	cout << "[cat] No such file or directory" << endl;
    //char error[] = "No such file or directory.\n";
    //write(1, error, sizeof error);
    exit(EXIT_FAILURE);
  }

  // read and write
  //rd = read(fd, buffer, size_of_file);
  rd = f_read(buffer, size_of_file, 1, fd);
  if (rd < 0)
  { // read into buffer
	cout << "[cat] Something is wrong!" << endl;
    //char err[] = "Something is wrong!\n";
    //write(1, err, sizeof err);
    exit(EXIT_FAILURE);
  }
  buffer[size_of_file + 1] = '\n';
  //wr = write(file_descriptor_to_be_written, buffer, size_of_file);
  wr = f_write(buffer, size_of_file, 1, file_descriptor_to_be_written);
  if (wr < 0)
  {
    // write from buffer to the stdout.
    cout << "[cat]Something goes wrong!" << endl;
	//char err[] = "Something is wrong!\n";
    //write(1, err, sizeof err);
    exit(EXIT_FAILURE);
  }
  if (f_close(fd) < 0)
  {
    //char error[] = "Can't close the file!";
    //write(1, error, sizeof error);
    cout << "[cat] Fail to close the file" << endl;
	exit(EXIT_FAILURE);
  }
}

// syscall verison
void microcat_using_syscall(const char *file_name, int file_descriptor_to_be_written)
{
  int fd, wr, rd;
  fd = f_open(string(file_name), "r");
  //(file_name, O_RDONLY);

  int size_of_file = get_file_size(string(file_name));
  char buffer[size_of_file + 1];

  // error checking and handling
  if (fd < 0)
  {
	cout << "[cat] No such file or directory" << endl;
    //char error[] = "No such file or directory.\n";
    //write(1, error, sizeof error);
    exit(EXIT_FAILURE);
  }

  // read and write
  //rd = read(fd, buffer, size_of_file);
  rd = f_read(buffer, size_of_file, 1, fd);
  if (rd < 0)
  { // read into buffer
	cout << "[cat] Something is wrong!" << endl;
    //char err[] = "Something is wrong!\n";
    //write(1, err, sizeof err);
    exit(EXIT_FAILURE);
  }
  buffer[size_of_file + 1] = '\n';
  wr = write(file_descriptor_to_be_written, buffer, size_of_file);
  if (wr < 0)
  {
    // write from buffer to the stdout.
    cout << "[cat]Something goes wrong!" << endl;
	//char err[] = "Something is wrong!\n";
    //write(1, err, sizeof err);
    exit(EXIT_FAILURE);
  }
  if (f_close(fd) < 0)
  {
    //char error[] = "Can't close the file!";
    //write(1, error, sizeof error);
    cout << "[cat] Fail to close the file" << endl;
	exit(EXIT_FAILURE);
  }
}

int microcat_calling(char **args, int argn) {
	// the case when there is no file input
	int standard_output = 1;
	int fd;
	if (argn == 1) {
		microcat_stdin_using_syscall(standard_output);
		return SUCCESS;
	}
	// read multiple text files
	for (int i = 1; i < argn; i++)
	{
		if (doubleRedirection(args, argn))
		{
			if (strncmp(args[i], ">>", sizeof(">>")) != 0 && i != 1 && i != (argn - 1))
			{
				// I assume that the txt file to be written into is to the right of '>'
				fd = f_open(args[argn - 1], "a");
				//fd = open(argv[argc - 1], O_RDWR | O_CREAT | O_APPEND, 0666);
				if (*args[i] == '-')
					microcat_stdin(fd);
				else
					microcat(args[i], fd);
			} else if (i == 1) {
				// delete all the content of the txt file that needs to be overwritten.
				//fd = open(argv[argc - 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
				fd = f_open(args[argn - 1], "a");
				microcat(args[i], fd);
			}
		}
		else if (IsRightRedirection(args, argn))
		{
			if (*args[i] != '>' && i != 1 && i != (argn - 1))
			{
				// I assume that the txt file to be written into is to the right of '>'
				fd = f_open(args[argn - 1], "a");
				//fd = open(argv[argc - 1], O_RDWR | O_CREAT | O_APPEND, 0666);
				if (*args[i] == '-')
					microcat_stdin(fd);
				else
					microcat(args[i], fd);
			}
			else if (i == 1)
			{
				// delete all the content of the txt file that needs to be overwritten.
				//fd = open(argv[argc - 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
				fd = f_open(args[argn - 1], "w");
				microcat(args[i], fd);
			}
		} else if (IsLeftRedirection(args, argn)) {
			microcat_using_syscall(args[argn - 1], standard_output);
		} else {
			// microcat all the text files if there is no redirection symbol.
			if (*args[i] == '-')
				microcat_stdin_using_syscall(standard_output);
			else
				microcat_using_syscall(args[i], standard_output);
		}
	}
}

// cat command implementation ends here

int check_built_in(Job *job)
{
	if (job->processList == NULL)
	{
		return FALSE;
	}
	char **args = job->processList->args;
	if (args == NULL)
	{
		return FALSE;
	}
	else if (args[0] == NULL)
	{
		return FALSE;
	}
	else if (strcmp(args[0], "kill") == 0)
	{
		return TRUE;
	}
	else if (strcmp(args[0], "fg") == 0)
	{
		return TRUE;
	}
	else if (strcmp(args[0], "bg") == 0)
	{
		return TRUE;
	}
	else if (strcmp(args[0], "jobs") == 0)
	{
		return TRUE;
	}
	// Homework 7 additional built-in commands
	else if (strcmp(args[0], "ls") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "chmod") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "mkdir") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "rmdir") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "cd") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "pwd") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "cat") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "more") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "rm") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "mount") == 0)
	{
	    printf("we get in mount! ya!\n");
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "umount") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else
		return FALSE;
}

int exeBuiltIn(char **args, int argn, sigset_t child_mask)
{
	if (strlen(curr_path) >= 128) {
		char* temp = pwd();
		free(temp);
	}
	if (strcmp(args[0], "kill") == 0)
	{
		bKill(args, argn);
	}
	else if (strcmp(args[0], "fg") == 0)
	{
		bFg(args, argn, child_mask);
	}
	else if (strcmp(args[0], "bg") == 0)
	{
		bBg(args, argn);
	}
	else if (strcmp(args[0], "jobs") == 0)
	{
		bJobs();
	}
	// Homework 7 additional built-in commands
	else if (strcmp(args[0], "ls") == 0)
	{
		return fs_ls(args, argn);
	}
	else if (strcmp(args[0], "chmod") == 0)
	{
	    return fs_chmod(args, argn); // this needs to be changed
	}
	else if (strcmp(args[0], "mkdir") == 0)
	{
	    return fs_mkdir(args, argn); // this needs to be changed
	}
	else if (strcmp(args[0], "rmdir") == 0)
	{
	    return fs_rmdir(args, argn); // this needs to be changed
	}
	else if (strcmp(args[0], "cd") == 0)
	{
	    return fs_cd(args, argn); // this needs to be changed
	}
	else if (strcmp(args[0], "pwd") == 0)
	{
	    return fs_pwd(); // this needs to be changed
	}
	else if (strcmp(args[0], "cat") == 0)
	{
	    return microcat_calling(args, argn);
	}
	else if (strcmp(args[0], "more") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "rm") == 0)
	{
	    if(argn == 2) {
	    	int result = fs_rm(args[1]);
	    	if(result == 0)
	    		return TRUE;
	    	else
	    		return FALSE;
	    }
	    else {
	    	printf("More than one pipe? We don't support any more");
	    	return FALSE;
	    }
		return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "mount") == 0)
	{
	    
	    return TRUE; // this needs to be changed
	}
	else if (strcmp(args[0], "umount") == 0)
	{
	    return TRUE; // this needs to be changed
	}
	else
	{
		perror("invalid input, check_built_in wrong probably\n");
		return FALSE;
	}
	return TRUE;
}

// not supporting pipe for now.
// executing the command without pipe. Example: emacs shell.c; or emacs shell.c &
void executing_command_without_pipe(Job *job, sigset_t child_mask)
{
	pid_t pid;
	int status;
	sigset_t child;
	//Job *childjob;
	char **args = job->processList->args;
	int argn = job->processList->argn;
	// check if the this job is built-in command, foreground, or background
	if (check_built_in(job))
	{
		job->status = JOBCOMP;
		exeBuiltIn(args, argn, child_mask);
		jobRemoveJobId(job->jobId);
	}
	// If it is foreground job
	else if (job->field == JOBFORE)
	{
		pid = fork();
		if (pid == 0)
		{
			setpgrp(); // set the pgid of the child process
			job->pgid = getpgrp();
			//child process
			signal(SIGINT, SIG_DFL);
			signal(SIGTERM, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			// execute the command
			tcsetpgrp(myShTerminal, getpid());
			signal(SIGTTOU, SIG_DFL);
			if ((execvp(args[0], args)) == FAIL)
			{
				printf("Didn't execute the command: %s! Either don't know what it is, or it is unavailable. \n", args[0]);
				exit(EXIT_FAILURE);
			}
		}
		else if (pid > 0)
		{
			// parent process
			setpgid(pid, 0); // set the pgid of the child process
			check_stat_pid = pid;
			tcsetpgrp(myShTerminal, pid); //bring child to foreground, modified Tue 7:25 pm
			job->pgid = pid;
			waitpid(pid, &status, WUNTRACED);
			// if the signal is termination (WIFSIGNALED) or normal exit, remove the job and free the memory.
			if (WIFSIGNALED(status) || WIFEXITED(status))
			{
				jobs_lock(child_mask);
				jobRemovePid(pid);
				jobs_unlock(child_mask);
				tcsetpgrp(myShTerminal, myShPGid);
				tcsetattr(myShTerminal, TCSADRAIN, &myShTmodes);
				return;
				//freeJob(job);
			}
			else if (WIFSTOPPED(status))
			{
				// else if the signal is stop, update the job's field, put it into background (save termios),
				job->field = JOBBACK;
				tcgetattr(myShTerminal, &job->j_Tmodes);
				/* Put the shell back in the foreground.  */
				tcsetpgrp(myShTerminal, myShPGid);

				/* Restore the shell’s terminal modes.  */
				tcsetattr(myShTerminal, TCSADRAIN, &myShTmodes);
				return;
			}
			else
				printf("Error in parent process");
		}
		else
		{
			perror("Forking error!");
			exit(EXIT_FAILURE);
		}
	}
	else if (job->field == JOBBACK)
	{
		// if this job is background job
		last_backgrounded = job->jobId;
		pid = fork();
		sigemptyset(&child);
		sigaddset(&child, SIGCHLD);
		sigprocmask(SIG_BLOCK, &child, NULL);
		if (pid == 0)
		{
			//child process
			setpgrp(); // set the pgid of the child process
			job->pgid = getpgrp();
			signal(SIGTSTP, SIG_DFL);
			signal(SIGINT, SIG_DFL);
			signal(SIGTERM, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			sigprocmask(SIG_UNBLOCK, &child, NULL);
			// execute the command
			if ((execvp(args[0], args)) == FAIL)
			{
				printf("Didn't execute the command: %s! Either don't know what it is, or it is unavailable. \n", args[0]);
				exit(EXIT_FAILURE);
			}
		}
		else if (pid > 0)
		{
			// parent process
			setpgid(pid, 0); // set the pgid of the child process
			check_stat_pid = pid;
			job->pgid = pid;
			sigprocmask(SIG_UNBLOCK, &child, NULL);
			// waitpid(pid, &status, WNOHANG);
			// // if the signal is termination (WIFSIGNALED) or normal exit, remove the job and free the memory.
			// if (WIFSIGNALED(status) || WIFEXITED(status)) {
			// 	jobs_lock(child_mask);
			// 	jobRemovePid(getpgid(pid));
			// 	jobs_unlock(child_mask);
			// 	//freeJob(job);
			// 	return;
			// } else
			// 	printf("Error in parent process");
		}
		else
		{
			perror("Forking error!");
			exit(EXIT_FAILURE);
		}
	}
}

void jobs_lock(sigset_t child_mask)
{
	sigprocmask(SIG_BLOCK, &child_mask, NULL);
}

void jobs_unlock(sigset_t child_mask)
{
	sigprocmask(SIG_UNBLOCK, &child_mask, NULL);
}
