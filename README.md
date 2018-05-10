# SuperTZJ-File-System
Name: Tianming (Mark) Xu, Jiaping Wang, Zhanpeng Wang, homework 7 File system

How to compile: 
	We have written a Makefile. If you want to compile the shell, you need to type in "make mysh" to compile it. The makefile will automatically compile fs.cpp to fs.o and then turn into libfs.so. If you only want to create the shared library, you can input "libfs.so" to create it. However, before you run any program used the thread library, you need to type in "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:." into terminal. 
	If you want to directly use libfs.so in your own program, you need to include "fs.h" in your program and remember to type in "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:." before you run the program.
  In the Testing folder, we have a bunch of tests to test the raw file system with hard code disk image. If you want to try any of them, just type "make nameoftest", you can have the executable file. The content of each test listed below.

How to run it:
	1. All the executable files,except format, need no argument, you just need to type in the name of test program you want to test to run it.
  2. To run format, you need to type in the name of the disk, and if you want to create DIY disk, you need to add the size of disk after the name of disk.

The difference between our file system and design doc:
	There are not many difference between our implementation and design. We add a bunch of helper functions to help us deal with intermediate step of reading and write to disk images, which are not included in our design doc. If we improve the existence of our superuser privilege, we need to have one more argument to identify whether the caller is superuser or not.

Known bugs and limitations
	limitations:
	1. Our file system still doesn not support Superuser privilege. Basically, the shell should have Superuser privilege so it can bypass the permission of writeonly and execute only file to print their status on the screen. The way we think to fix this problem is to add an extra argument to know whether it is superuser who calls the function. If it is superuser, then close all permission checks.
	2. The redirection can not work generally in our file system, the redirection depends on cat command to work. So it is hard to test big file. We provide a test_large.cpp test case in our Testing folder to cover this case though.
	3.In the file name, we can not let user create file name with "/", because that is the special symbol for directory. The file can be created, but without "/".
	4. We didn't have time to deal with memory leak, and there are several minor invalid read in memory.
	5. We didn't test the i3block. So it might be some problems in our i3block.


file directory:
	The file system program needs fs.cpp, fs.h and file_util.h. 
	The shell needs ll.h ll.c shell.c parser.h parser.c util.h run_command.h run_command.c and format.cpp.
	The tests are all in Testing folder. all tests need to compile test_util and test_util.cpp and fs.cpp to make them work.
	we collaborate with Eileen Feng, Ziting Shen Elieen Feng and got inspired by these outside resources:
	https://github.com/stpddream/OSHomework/tree/master
	https://blog.csdn.net/heikefangxian23/article/details/51579971
	https://blog.csdn.net/u013007900/article/details/50395940
	https://blog.csdn.net/tianqingdezhuanlan/article/details/51344739 -- file system design
	https://blog.csdn.net/little_qi/article/details/73801336
	https://wizardforcel.gitbooks.io/wangdaokaoyan-os/content/17.html  --description
	http://www.java-samples.com/showtutorial.php?tutorialid=572 fopen implementation
	https://github.com/mattrothberg1/303hw3 fat implementation
	https://github.com/windcode/os_filesystem/blob/master/MingOS.h inode file system, incomplete
	https://wenku.baidu.com/view/dcc7d42d7375a417866f8ff4.html?sxts=1524584589896 
	http://bbs.chinaunix.net/thread-1978635-1-1.html   mini file system implementation
	https://stackoverflow.com/questions/28493392/executing-redirection-and-in-custom-shell-in-c supporting redirection
	https://stackoverflow.com/questions/28490290/how-can-i-pass-the-redirection-operator-as-an-argument-for-execv supporting redirection 2
	https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/rtread.htm readdir explained

Test Desscription:
     We designed ten night cases to test the integrity of my file system library. Most of them use a hard coded disk image named "test", I create this disk image at the beginning of the test and add or delete stuffs from it. The tests don't have expected performance, but we print most of useful printing message on the screen to detect problems.

How to run test file:
    In makefile, we have correspond command to compile any of my test case. You just need to input "make name" and replace name by the test case you want to test. After compiling, you can run it by "./name", replacing name by the test case name.

Logic of test cases:
      we will briefly introduce what we want to test in each of my test case:
      1. test_format.cpp
      	 We create a specific disk image using format_default_size or format_with_given_size, and print out the superblock and inode region of it to see what is inside.
      2. test_helper.cpp
      	 This test is testing whether one of our important helper function works, not testing the file system itself.
      3. test_open.cpp
      	 This test is testing f_open of our file system. In the f_open, there are several choices you can test, open a hard coded small file with read mode, open a not existed file with write mode, and open a hard coded middle size file with read mode or append mode. Uncomment the parts you want to test.
      4. test_write.cpp
      	 This test is testing the robustness of f_write function. We tried to write in a existed file and non existed file here.
      5. test_large.cpp
      	 An extended version of test_write. Here we test whether we are able to write into i2 block in our file system. You can change the number of blocks when you test, but the behavior will be uncontrollable if you input size larger than the disk image size.
      6. test_fseek.cpp
      	 This test is a combination of testing f_read and f_seek. We tried to f_seek to a specific position and then read from the file. 
      7. test_remove.cpp
         This test is testing wether the f_remove works. We tried to remove a small file and a large file created by f_write. 
      8. test_dir.cpp
      	 This test tests the robustness of our dir relevant functions. We create a bunch of directories and try to print out the entry of root directory to see whether the directories have been added. 
	 
	 Hopefully the tests can be helpful! We have a very tough semester but really learn a lot from this course. Thank you Dianna and definitely thank you Rachel to answer us so many questions! Hope you have a good time in Google and remember to visit us in the future. (Or probably meet you in San Jose? :) ) 
