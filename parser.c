/*http://blog.csdn.net/lin535061851/article/details/48395607
void trimLeft(char *s)  
{  
    int i=0, j=0;  
    //传入空值则退出  
    if(!strlen(s)) return;  
    //找到首个不为空的字符  
    while( (s[i] == ' ' || s[i] == '\n' || s[i] == '\t') && s[i] != '\0') i++;  
    //从i位置的字符开始左移i个位置  
    while( s[i] != '\0') s[j++] = s[i++];  
    s[j] = '\0';  
}  

// trim all \t \n space to the right of the string
void trimRight(char *s)  
{  
    int pos;  
    pos = strlen(s) - 1;  
    //若尾部字符为空，则将其设置为末尾字符  
    while( s[pos] == ' ' || s[pos] == '\n' || s[pos] == '\t') {  
        s[pos--] = '\0';  
        if(pos<0) break;  
    }  
} 

void trimAll(char *s) {
	trimRight(s);
	trimLeft(s);
} */
#include "parser.h"

int check_last_character_in_process(char *process_line)
{
	int len = strlen(process_line) - 1;
	while (process_line[len] != '\0')
	{
		if (process_line[len] == ' ' || process_line[len] == '\t' || process_line[len] == '\n')
		{
			len--;
		}
		else if (process_line[len] == '&')
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}

// return -1 if it is not the last process in the line but has '&' sign as the last nonspace character
int parseCommands(char *line, char **command)
{
	int count = 0;
	char *split;

	if ((split = strtok(line, ";")) == NULL)
	{
		// count = 1;
		// command[count-1] = malloc(strlen(line)+1);
		// bzero(command[count-1], strlen(line)+1);
		// strcpy(command[count-1], line);
		return count;
	}
	int len;
	while (split != NULL)
	{
		len = strlen(split);
		command[count] = malloc(len + 1);
		bzero(command[count], len + 1);
		strcpy(command[count], split);
		command[count][len] = '\0';
		count++;
		split = strtok(NULL, ";");
		/*if(count >= num_process) {
				i++;
				num_process = DEFAULT_NUM_PROCESS << i;
				*command = realloc(*command, num_process * sizeof(char*));
		}*/
	}
	command[count] = NULL;
	for (int j = 0; j < count - 1; ++j)
	{
		if (check_last_character_in_process(command[j]))
		{
			return -1;
		}
	}

	return count;
}

int parseSegments(char *command, char **segments)
{
	int count = 0;
	//int i = 0;
	//int num_segments = DEFAULT_NUM_SEGMENTS;
	char *split;

	if ((split = strtok(command, "|")) == NULL)
	{
		int llen = strlen(command);
		segments[count] = malloc(llen + 1);
		bzero(segments[count], llen + 1);
		strcpy(segments[count], command);
		count = 1;
		return count;
	}
	int len;
	while (split != NULL)
	{
		len = strlen(split);
		segments[count] = malloc(len + 1);
		bzero(segments[count], len + 1);
		strcpy(segments[count], split);
		segments[count][len] = '\0';
		count++;
		split = strtok(NULL, "|");
		/*if(count >= num_segments) {
				i++;
				num_segments = DEFAULT_NUM_SEGMENTS << i;
				segments = realloc(segments, num_segments * sizeof(char*));
		}*/
	}

	return count;
}

int parseArguments(char *segments, char **arguments)
{
	int count = 0;
	//int i = 0;
	int foreground = 1; //  will change to -1 if it's in background
	//int num_arguments = DEFAULT_NUM_ARG;
	char *split;

	if ((split = strtok(segments, " \n\t")) == NULL)
	{
		// int llen = strlen(segments);
		// arguments[count] = malloc(llen+1);
		// bzero(arguments[count], llen+1);
		// strcpy(arguments[count], segments);
		// if (llen != 0) {
		// 	count++;
		// }
		return count;
	}
	int len;
	while (split != NULL)
	{
		len = strlen(split);
		arguments[count] = malloc(len + 1);
		bzero(arguments[count], len + 1);
		strcpy(arguments[count], split);
		arguments[count][len] = '\0';
		count++;
		split = strtok(NULL, " \n\t");
		/*if(count >= num_arguments) {
				i++;
				num_arguments = DEFAULT_NUM_ARG << i;
				segments = realloc(segments, num_arguments * sizeof(char*));
		}*/
	}
	arguments[count] = NULL;
	int temp = strlen(arguments[count - 1]);
	if (temp == 1 && arguments[count - 1][temp - 1] == '&')
	{
		foreground = -1;
		free(arguments[count - 1]);
		arguments[count - 1] = NULL;
		count--;
	}
	else if (arguments[count - 1][temp - 1] == '&')
	{
		foreground = -1;
		arguments[count - 1][temp - 1] = '\0';
	}

	return count * foreground;
}
