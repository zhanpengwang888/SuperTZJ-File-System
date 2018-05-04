#ifndef PARSE_
#define PARSE_

#include "util.h"

char* readLine();
int parseCommands(char* line, char** command);
int parseSegments(char* command, char** segments);
int parseArguments(char* segments, char** arguments);
int check_last_character_in_process(char* process_line);

#endif
