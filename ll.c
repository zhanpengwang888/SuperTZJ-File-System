#include "ll.h"

void jobInit()
{
	head = malloc(sizeof(Node));
	tail = malloc(sizeof(Node));
	head->prev = NULL;
	head->next = tail;
	tail->prev = head;
	tail->next = NULL;
	head->job = NULL;
	tail->job = NULL;
	id = 1;
}

Job *createJob(char *line, Process *process, int status, int field)
{
	Job *toReturn = malloc(sizeof(Job));
	toReturn->status = status;
	toReturn->jobId = id;
	toReturn->field = field;
	id++;
	if (line != NULL)
	{
		toReturn->line = malloc(sizeof(char) * strlen(line) + 1);
		bzero(toReturn->line, strlen(line) + 1);
		strcpy(toReturn->line, line);
	}
	toReturn->processList = process;
	toReturn->pgid = 0;
	return toReturn;
}

Job *getJobJobId(int job_id)
{
	Node *temp = head->next;
	while (temp != NULL && temp->job != NULL)
	{
		if (temp->job->jobId == job_id)
		{
			return temp->job;
		}
		else
		{
			temp = temp->next;
		}
	}
	return NULL;
}

Job *getJobPid(pid_t pid)
{
	Node *temp = head->next;
	while (temp != NULL && temp->job != NULL)
	{
		if (temp->job->pgid == pid)
		{
			return temp->job;
		}
		else
		{
			temp = temp->next;
		}
	}
	return NULL;
}

Job *getJobCommandName(char *command)
{
	Node *temp = head->next;
	int count = 0;
	Job *toReturn;
	while (temp != NULL && temp->job != NULL)
	{
		if (strstr(temp->job->line, command) != NULL)
		{
			toReturn = temp->job;
			count++;
		}
		else
		{
			temp = temp->next;
		}
	}
	return (count > 1 ? NULL : toReturn);
}

Job *getJLastSuspended()
{
	return getJobJobId(last_suspended);
}

Job *getJLastBackgrounded()
{
	return getJobJobId(last_backgrounded);
}

int jobInsert(Job *job)
{
	Node *toAdd = malloc(sizeof(Node));
	if (!toAdd)
	{
		return FALSE;
	}
	toAdd->job = job;
	Node *temp = tail->prev;
	temp->next = toAdd;
	toAdd->prev = temp;
	toAdd->next = tail;
	tail->prev = toAdd;
	return TRUE;
}

int jobRemovePid(pid_t pid)
{
	Node *temp = head->next;
	while (temp != NULL && temp->job != NULL)
	{
		if (temp->job->pgid == pid)
		{
			Node *temp2 = temp->prev;
			Node *temp3 = temp->next;
			temp2->next = temp3;
			temp3->prev = temp2;
			int i = temp->job->jobId;
			// update all job id after the deleted one
			while (temp3!= NULL && temp3->job != NULL)
			{
				if (temp3->job->jobId > i)
				{
					temp3->job->jobId--;
				}
				temp3 = temp3->next;
			}
			id--;
			freeJob(temp->job);
			temp->job = NULL;
			free(temp);
			temp = NULL;
			return TRUE;
		}
		else
		{
			temp = temp->next;
		}
	}
	return FALSE;
}

int jobRemoveJobId(int job_id)
{
	Node *temp = head->next;
	while (temp != NULL && temp->job != NULL)
	{
		if (temp->job->jobId == job_id)
		{
			Node *temp2 = temp->prev;
			Node *temp3 = temp->next;
			temp2->next = temp3;
			temp3->prev = temp2;
			int i = temp->job->jobId;
			// update all job id after the deleted one
			while (temp3 != NULL && temp3->job != NULL)
			{
				if (temp3->job->jobId > i)
				{
					temp3->job->jobId--;
				}
				temp3 = temp3->next;
			}
			id--;
			freeJob(temp->job);
			temp->job = NULL;
			free(temp);
			temp = NULL;
			return TRUE;
		}
		else
		{
			temp = temp->next;
		}
	}
	return FALSE;
}

void jobSetPGid(Job *job, pid_t pgid)
{
	job->pgid = pgid;
	return;
}

void jobChangeStatus(Job *job, int status)
{
	job->status = status;
	return;
}

void printList()
{
	Node *temp = head->next;
	while (temp != NULL && temp->job != NULL)
	{
		if (temp->job->status == JOBCOMP)
		{
			printf("[%d] Done                     %s\n", temp->job->jobId, temp->job->line);
		}
		else if (temp->job->status == JOBSTOP)
		{
			printf("[%d] Stopped                     %s\n", temp->job->jobId, temp->job->line);
		}
		else if (temp->job->status == JOBRUN)
		{
			printf("[%d] Run                     %s\n", temp->job->jobId, temp->job->line);
		}
		else if (temp->job->status == JOBTERM)
		{
			printf("[%d] Forcefully Terminated                     %s\n", temp->job->jobId, temp->job->line);
		}
		else
		{
			perror("Print status error!\n");
			exit(EXIT_FAILURE);
		}
		temp = temp->next;
	}
}

void printJobStatus(Job *job)
{
	if (job->status == JOBCOMP)
		printf("\n[%d] Done                     %s\n", job->jobId, job->line);
	else if (job->status == JOBSTOP)
		printf("\n[%d] Stopped                     %s\n", job->jobId, job->line);
	else if (job->status == JOBRUN)
		printf("\n[%d] Run                     %s\n", job->jobId, job->line);
	else if (job->status == JOBTERM)
		printf("\n[%d] Forcefully terminated                     %s\n", job->jobId, job->line);
	else
	{
		perror("Print status error!\n");
		exit(EXIT_FAILURE);
	}
}

void freeArgs(char **args)
{
	int count = 0;
	while (args != NULL && args[count] != NULL)
	{
		free(args[count]);
		args[count] = NULL;
		count++;
	}
	//args = NULL;
	free(args);
	args = NULL;
}

void freeProcess(Process *process)
{
	Process *temp;
	while (process != NULL)
	{
		temp = process;
		process = process->next;
		freeArgs(temp->args);
		temp->args = NULL;
		free(temp);
		temp = NULL;
	}
}

void freeJob(Job *job)
{
	if (job != NULL && job->processList != NULL)
	{
		freeProcess(job->processList);
		job->processList = NULL;
	}
	if (job != NULL && job->line != NULL)
	{
		free(job->line);
		job->line = NULL;
	}
	if (job != NULL)
	{
		free(job);
		job = NULL;
	}
}

void freeJobList()
{
	while (head->next->job != NULL)
	{
		Node *temp = head->next;
		head->next = temp->next;
		freeJob(temp->job);
		temp->job = NULL;
		free(temp);
		temp = NULL;
	}
	free(head);
	free(tail);
}
