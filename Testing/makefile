# test_format: test_format.cpp test_util.h
# 	g++ -o test_format test_format.cpp test_util.c
test_format: test_format.cpp ../file_util.h test_util.h
	g++ -std=c++11 -o test_format test_format.cpp ../fs.cpp test_util.cpp 
test_open: test_open.cpp ../file_util.h test_util.h
	g++ -std=c++11 -o test_open test_open.cpp ../fs.cpp test_util.cpp 
test_helper.cpp: test_helper.cpp ../file_util.h test_util.h
	g++ -std=c++11 -o test_helper test_helper.cpp test_util.cpp

SHELL:=/bin/bash -O extglob
clean:
	@rm -f !(*.c|*.cpp|makefile|README.txt|*.h)