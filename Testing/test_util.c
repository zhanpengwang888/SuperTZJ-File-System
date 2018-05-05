#include "test_util.h"


void print_inode(inode* inode,int index, int detail){
  char status[20] = "";
  if(inode->next_inode) {
    strcpy(status,"FREE");
  }
  else {
    strcpy(status,"USED");
  }
  printf("Inode %d is %s, the file size is %d\n",index, status, inode->size);
    if(detail) {        
        printf("Direct blocks: {");
        for(int i = 0; i < N_DBLOCKS; i++) {
            printf("%d, ", inode->dblocks[i]);
        }
        printf("}\n");               
        
        printf("Indirect blocks: {");
        for(int i = 0; i < N_IBLOCKS; i++) {
            printf("%d, ", inode->iblocks[i]);
        }
        printf("}\n");               
	
    }
}

void print_superblock(Superblock* sp) {
  printf("the block size is %d\n",sp->size);
  printf("the inode_offset is %d\n",sp->inode_offset);
  printf("the data_offset is %d\n",sp->data_offset);
  printf("the swap_offset is %d\n",sp->swap_offset);
  printf("the first free node is %d\n", sp->free_inode);
  printf("the first free block is %d\n", sp->free_block);
  printf("the inode region starts at %d\n", inode_start);
  printf("the inode region ends at  %d\n", inode_end);
}

Inode* next_inode(inode* inode) {
  return (inode*)((char*)inode + INODE_SIZE); 
}