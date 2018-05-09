mysh: shell.c run_command.h util.h parser.h ll.h libfs.so
	g++ -std=c++11 -Wall -pedantic -g -o mysh shell.c run_command.c parser.c ll.c -L. -lfs -lreadline
#export LD_LIBRARY_PATH=.
#create library.o file                                                                                       
fs.o :fs.cpp
	g++ -Wall -fpic -fpack-struct -Dworst -c fs.h fs.cpp
#create the shared library                                                                                   
libfs.so: fs.o
	g++ -o libfs.so fs.o -shared

format: format.cpp fs.h Testing/test_util.h
	g++ -std=c++11 -g -o format_disk format.cpp fs.cpp Testing/test_util.cpp

# clean all the executable and object files                                                                                                                                     
SHELL:=/bin/bash -O extglob
clean:
	@rm -f !(*.c|Makefile|README.md|*.h|democode|presentation)
