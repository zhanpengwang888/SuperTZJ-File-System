# SuperTZJ-File-System
Name: Tianming (Mark) Xu, Jiaping Wang, Zhanpeng Wang, homework 7 File system

How to compile: 
	We have written a Makefile. If you want to compile the shell, you need to type in "make mysh" to compile it. The makefile will automatically compile fs.cpp to fs.o and then turn into libfs.so. If you only want to create the shared library, you can input "libfs.so" to create it. However, before you run any program used the thread library, you need to type in "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:." into terminal. 
	If you want to directly use libfs.so in your own program, you need to include "fs.h" in your program and remember to type in "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:." before you run the program.
  In the Testing folder, we have a bunch of tests to test the raw file system with hard code disk image. If you want to try any of them, just type "make nameoftest", you can have the executable file. The content of each test listed below.

How to run it:
	1. All the executable files,except format, need no argument, you just need to type in the name of test program you want to test to run it.
  2. To run format, you need to type in the name of the disk, and if you want to create DIY disk, you need to add the size of disk after the name of disk.

Logic of our file system:
	This is a really huge program and hard to describe its logic in some sentences. Please read our design

Known bugs and limitations
	bugs:
	 There is no known bug currently. 

	limitations:
		1. My mem library allocates memory by considering the worst case of user usage. That is to say, if user requests 400 bytes, we will predict user might request 50 8-bytes chunk of memory. Since we need to have a 32-bytes header in each block, we need to allocate 50 * (32 + 8) = 2000 bytes. But we will have a global remaining space value to keep track of the remaining space user can use. So even though we allocate more spaces to user, but they can just use the part they request.
    2. In my mem Library, I have five global varaibles, one more than requirement. The reason is that I need to have the head of two lists: allocated list and free list. And I need a global flag to know whether we should globally coalesce or locally coalesce. The other two, one stores the total chunk of memory I mmap, in order to calculate the end of my block. The last one is to store the remaining space of user can use. The other two store the current address of largest free block and second largest free block. This design can help us find the worst fit free block faster than normal solution.Since I didn't use global variables to do some fancy bookkeeping stuff, I hope they are reasonable and not be taken off too much point.
    3. In global coalesce method, I traverse the whole list to coalesce every possible free blocks. So the performance will be not very good if we need to do global coalesce many times when the free list is long.


file directory:
	The malloc program needs mem.c and mem.h. I collaborate with Zhanpeng Wang, Jiaping Wang, Elieen Feng and Ziting Shen and got inspired by this GitHub page:
https://github.com/stpddream/OSHomework/tree/master

Test Desscription:
     I designed ten test cases to test the integrity of my malloc library.
     There are six normal tests to see how my thread library works when they are used correctly. The other four tests are testing what will happen when the library is used incorrectly. My goal is to make sure the normal tests can have expected result and misuse cases not crash.

How to run test file:
    In my makefile, I have correspond command to compile any of my test case. You just need to input "make name" and replace name by the test case you want to test. After compiling, you can run it by "./name", replacing name by the test case name.

Logic of test cases:
      I will briefly introduce what I want to test in each of my test case:
      1. check_8_align.c
      	 I allocate two chunks of memory and print their address and result of modding with 8. And also print the difference between them.
      2. simple_alloc.c
      	 This test is testing whether libmem is able to allocate one 8-byte chunk.
      3. few_aligned_alloc.c
      	 This test is testing whether libmem is able to allocate 10 8-bytes chunks of memory.
      4. odd_aligned_alloc.c
      	 This test is testing whether libmem is able to allocate 10 odd-number (related to their index) size of chunk of memory.
      5. bad_args.c
      	 This test I used to test whether Mem_Init fail when the input argument is invalid. After that wrong initialization, I call Mem_alloc to make sure there should not crash anything.
      6. intensive.c
      	 This test modified from Kyu's two-million-with-coalesce, I try to free all the blocks after allocation. It will take a little bit longer than the original test.
      7. worst_fit.c
         This test is testing whether we fit the user request into the largest block of memory we have. I allocate a series of memory, free some, and allocate one more. The last one should be allocated into the largest piece of memory, instead of filling in the blocks I just freed.
      8. coalesce.c
      	 This test tests what will happen when we switch between freeing with coalesce or without coalesce. 
      9-11. simple_alloc_free.c aligned_alloc_free.c odd_alloc_free.c
      	 I free everything after allocating them. The structure previous free should be same as the allocate test.
      12. init_one_page.c
      	  In this test I test what will happen when user requests exactly one page of memory.
      13. init_round_page.c
          In this test I test what will happen when the user request less than a page of memory but we round up to at least one page.
      14. no_space.c
        This program tests what happen when user runs out of her requested memory in Mem_Init. I will warn her and no longer allocate 
        memory to her before user frees her allocated block.
      15. alloc_used.c
        This program we write several strings into the allocated blocks and print them. No memory error should be shown if you run it in valgrind. And the content of strings should be printed correctly.
