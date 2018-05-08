#include "test_util.h"

using namespace std;

void update_sp() {
 fseek(fp,BOOT_SIZE,SEEK_SET);
 fwrite(sp,sizeof(Superblock),1,fp);
}
void create_mid_node(inode* test_inode, int index, int size, string name){
 test_inode->nlink = 1;
 test_inode->permission = 7;
 test_inode->type = NORMAL_FILE;
 test_inode->next_inode = -1;
 test_inode->size = size;
 test_inode->uid = 0;
 test_inode->gid = 0;
 test_inode->parent = 0; //parent is root
 for (int i = 0; i < N_DBLOCKS; i++)
 {
     test_inode->dblocks[i] = i + 1;
 }

 for (int i = 0; i < N_IBLOCKS; i++)
 {
   test_inode->iblocks[i] = -1;
 }

 test_inode->i2block = -1;
 test_inode->i3block = -1;
 //can't directly assign, use strcpy
 //default_inode->file_name = "";
 std::strcpy(test_inode->file_name, name.c_str());

}

void create_node(inode* test_inode, int index, int size, string name){
 test_inode->nlink = 1;
 test_inode->permission = 7;
 test_inode->type = NORMAL_FILE;
 test_inode->next_inode = -1;
 test_inode->size = size;
 test_inode->uid = 0;
 test_inode->gid = 0;
 test_inode->parent = 0;
 for (int i = 0; i < N_DBLOCKS; i++)
 {
   if(i == 0)
     test_inode->dblocks[i] = 1;
   else
     test_inode->dblocks[i] = -1;
 }

 for (int i = 0; i < N_IBLOCKS; i++)
 {
   test_inode->iblocks[i] = -1;
 }

 test_inode->i2block = -1;
 test_inode->i3block = -1;
 //can't directly assign, use strcpy
 //default_inode->file_name = "";
 std::strcpy(test_inode->file_name, name.c_str());

}

void create_mid_file(char* name) {
 fp = fopen(name,"r+");
 if(fp == NULL)
   perror("Error\n");
 fseek(fp, 0, SEEK_END);
   size_t size = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   file_buffer = (char*)malloc(sizeof(char) * size + 1);
 int t_size = fread(file_buffer, size, 1,fp);
   sp = (Superblock*)(file_buffer + BOOT_SIZE);
 inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
    inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
    inode_num = (inode_end - inode_start) / INODE_SIZE;
    //print root directory's entries
    inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;

    //create the test file and inode -- not write in yet 
    char* test_block = (char*) malloc(BLOCK_SIZE);
 strcpy(test_block,junk_c);
 int s_size = strlen(test_block);
 inode *test_inode = (inode *)malloc(sizeof(inode));
 char* f_name = "test.txt";
 create_mid_node(test_inode,1,s_size*10,"test.txt");
 //write test inode into disk
 fseek(fp,inode_start+1*INODE_SIZE,SEEK_SET);
 fwrite(test_inode,INODE_SIZE,1,fp);
 //put the test blocks list into file
 int data_address;
 for(int i = 1; i < 11; i++) {
   data_address = inode_end + i * BLOCK_SIZE;
   fseek(fp,data_address,SEEK_SET);
   fwrite(test_block,BLOCK_SIZE,1,fp);
 }

 //change root directory entry
 int root_address = inode_end;
 if(fseek(fp, root_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
   }
   char* root_block = (char*)(malloc(BLOCK_SIZE));
   fread(root_block,BLOCK_SIZE,1,fp);
   directory_entry* root_table = (directory_entry*)(root_block);
   strcpy(root_table[2].file_name, f_name);
   root_table[2].inode_entry = 1;
   //cout << root_table[2].file_name << root_table[2].inode_entry << endl;
   //cout << root_table[1].file_name << root_table[1].inode_entry << endl;
 if(fseek(fp, root_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
   }
   fwrite(root_block,BLOCK_SIZE,1,fp);
   //update root inode
   root-> size += sizeof(directory_entry);
   fseek(fp,inode_start,SEEK_SET);
   fwrite(root,INODE_SIZE,1,fp);


   //update superblock
   sp->free_inode = 2;
   sp->free_block = 11;
   //print_superblock(sp);
   update_sp();
   print_superblock(sp);
   fclose(fp);
}
//get a disk image and modify to be test disk image
void create_test_file(char* name) {
 fp = fopen(name,"r+");
 if(fp == NULL)
   perror("Error\n");
 fseek(fp, 0, SEEK_END);
   size_t size = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   cout << "the size of file is " << size <<endl;
   file_buffer = (char*)malloc(sizeof(char) * size + 1);
 int t_size = fread(file_buffer, size, 1,fp);
   sp = (Superblock*)(file_buffer + BOOT_SIZE);
 print_superblock(sp);
 inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
    inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
    inode_num = (inode_end - inode_start) / INODE_SIZE;
    //print root directory's entries
    inode* inode_head = (inode*)(file_buffer + inode_start);
    inode* root = inode_head;
    //print_inode(root,0,1);
    //print_directory(sp,root,file_buffer);
 //    for(int i = 0; i < inode_num ; i++) {
 //       fseek(fp,inode_start + i * INODE_SIZE,SEEK_SET);
 //     disk_inode_list[i] = (inode*)(malloc(INODE_SIZE));
 //     fread(disk_inode_list[i],INODE_SIZE,1,fp);
 //     //print_inode(disk_inode_list[i],i,1);
 // }
    //create test file
 char* test_block = (char*) malloc(BLOCK_SIZE);
 strcpy(test_block,"Mark is the best!");
 int s_size = strlen(test_block);
 inode *test_inode = (inode *)malloc(sizeof(inode));
 char* f_name = "test.txt";
 create_node(test_inode,1,s_size,"test.txt");
 //print_inode(test_inode,1,1);
 //write inode into disk
 fseek(fp,inode_start+1*INODE_SIZE,SEEK_SET);
 fwrite(test_inode,INODE_SIZE,1,fp);
 int data_address = inode_end + 1 * BLOCK_SIZE;
 fseek(fp,data_address,SEEK_SET);
 fwrite(test_block,BLOCK_SIZE,1,fp);

 //change root directory entry
 int r_address = inode_end;
 if(fseek(fp, r_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
   }
   char* root_block = (char*)(malloc(BLOCK_SIZE));
   fread(root_block,BLOCK_SIZE,1,fp);
   directory_entry* root_table = (directory_entry*)(root_block);
   strcpy(root_table[2].file_name, f_name);
   root_table[2].inode_entry = 1;
   //cout << root_table[2].file_name << root_table[2].inode_entry << endl;
   //cout << root_table[1].file_name << root_table[1].inode_entry << endl;
 if(fseek(fp, r_address,SEEK_SET) < 0) {
      perror("lseek fails\n");
      return;
   }
   fwrite(root_block,BLOCK_SIZE,1,fp);
   //update root inode
   root-> size += sizeof(directory_entry);
   fseek(fp,inode_start,SEEK_SET);
   fwrite(root,INODE_SIZE,1,fp);
   //refresh buffer -- for testing
   fseek(fp, 0, SEEK_SET);
   t_size = fread(file_buffer, size, 1,fp);
   print_directory(sp,root,file_buffer);

   //print the test_block
   // fseek(fp,data_address,SEEK_SET);
   // fread(test_block,BLOCK_SIZE,1,fp);
   // cout << test_block << endl;
  //read from the file descriptor
  
   //update the superblock into the disk
   sp->free_inode = 2;
   sp->free_block = 2;
   //print_superblock(sp);
   update_sp();

   //test update_sp();
   /*
   fclose(fp);
   fp = fopen("test","r+");
   fseek(fp, 0, SEEK_SET);
   t_size = fread(file_buffer, size, 1,fp);
   sp = (Superblock*)(file_buffer + BOOT_SIZE);
   print_superblock(sp);
   */
   fclose(fp);


}
void print_inode(inode* Inode,int index, int detail){
  char status[20] = "";
  if(Inode->nlink == 0) {
    strcpy(status,"FREE");
  }
  else if(Inode->nlink > 0){
    strcpy(status,"USED");
  }
  else {
    strcpy(status,"UNKNOWN");
  }
  printf("Inode %d is %s, the file size is %ld\n",index, status, Inode->size);
    if(detail) {        
        printf("Direct blocks: {");
        for(int i = 0; i < N_DBLOCKS; i++) {
            printf("%d, ", Inode->dblocks[i]);
        }
        printf("}\n");               
        
        printf("Indirect blocks: {");
        for(int i = 0; i < N_IBLOCKS; i++) {
            printf("%d, ", Inode->iblocks[i]);
        }
        printf("}\n");               
	
    }
}

void print_superblock(Superblock* sp) {
  printf("the block size is %d\n",sp->size);
  printf("the inode_offset is %d\n",sp->inode_offset);
  printf("the data_offset is %d\n",sp->data_offset);
  printf("the first free node is %d\n", sp->free_inode);
  printf("the first free block is %d\n", sp->free_block);
  printf("the root directory is %d\n",sp->root);
}

inode* next_inode(inode* Inode) {
  return (inode*)((char*)Inode + INODE_SIZE); 
}

void print_inode_region(Superblock* sp, char* buffer) {
    int inode_start = BOOT_SIZE + SUPER_SIZE + sp->inode_offset * BLOCK_SIZE;
    int inode_end = BOOT_SIZE + SUPER_SIZE + sp->data_offset * BLOCK_SIZE;
    int inode_num = (inode_end - inode_start) / INODE_SIZE;
    printf("inode_end is %d\n",inode_end);
    printf("inode_start is %d\n",inode_start);
    printf("there should be %d\n", (inode_end - inode_start));
    printf("INODE_SIZE is %lu\n",INODE_SIZE);
    printf("inode num is %d\n",inode_num);
    inode* inode_head = (inode*)(buffer + inode_start);
    inode* cur = inode_head;
    for(int i = 0; i < inode_num; i++) {
      print_inode(cur,i,1);
      cur = next_inode(cur);
  }
}

//first version only support dblock area
void print_directory(Superblock* sp, inode* dir_node, char* buffer) {
  unsigned long remaining_size = dir_node->size;
  if(remaining_size == 0) {
    cout<< "Empty directory, wrong!" <<endl;
    return;
  }
  if(dir_node->type != DIRECTORY_FILE){
    cout<< "Not a dir" <<endl;
    return;
  }
  int data_offset = sp->data_offset;  // get the data offset from the super block
  size_t data_region_starting_addr = BOOT_SIZE + SUPER_SIZE + data_offset * BLOCK_SIZE;
  //cout << "we are here" << endl;
  for (int i = 0; i < N_DBLOCKS; i++)
  {
    int data_block_index = dir_node->dblocks[i];
    cout <<"the block index is " << data_block_index << endl;
    // one data block can have 16 directory entries
    size_t directories_starting_region = data_region_starting_addr + BLOCK_SIZE * data_block_index;
    //cout << "the address we use is " << directories_starting_region << endl;
    for (int j = 0; j < BLOCK_SIZE / sizeof(directory_entry); j++)
    {
      // may not work! Logic seems to be fine.
      //lseek(buffer, directories_starting_region + j * sizeof(directory_entry), SEEK_SET);
      //char *entry_char = (char *)malloc(sizeof(directory_entry));
      //char entry_char[sizeof(directory_entry)];
      //read(disk, entry_char, sizeof(directory_entry));
      directory_entry *entry = (directory_entry *)((char*)buffer + directories_starting_region + j * sizeof(directory_entry));
      //directory_entry *entry = (directory_entry *)disk_buffer + directories_starting_region + j * sizeof(directory_entry); // get each directory entry
      cout << "the " << j << "th entry has name " <<  entry->file_name << " and inode number is " << entry->inode_entry <<endl;
      remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes. 
      if (remaining_size <= 0)
      {
        return;
      }
      //remaining_size -= sizeof(directory_entry); // reducing the remaining size by 32 bytes.
      cout<< "the remaining size is " << remaining_size <<endl;
    }
  }

}
