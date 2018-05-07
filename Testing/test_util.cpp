#include "test_util.h"

using namespace std;
void print_inode(inode* Inode,int index, int detail){
  char status[20] = "";
  if(Inode->next_inode >= 0) {
    strcpy(status,"FREE");
  }
  else if(Inode->next_inode == -1){
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
    cout << "the address we use is " << directories_starting_region << endl;
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
