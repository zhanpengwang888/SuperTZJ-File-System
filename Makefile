mysh: shell.c run_command.h util.h parser.h ll.h
	gcc -Wall -pedantic -g -o mysh shell.c run_command.c parser.c ll.c -lreadline
format: format.cpp fs.h Testing/test_util.h
	g++ -g -o format_disk format.cpp fs.cpp Testing/test_util.cpp

# clean all the executable and object files                                                                                                                                     
SHELL:=/bin/bash -O extglob
clean:
	@rm -f !(*.c|Makefile|README.md|*.h|democode|presentation)
