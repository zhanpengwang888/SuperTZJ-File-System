#include "test_util.h"


void print_inode(inode* Inode,int index, int detail){
  char status[20] = "";
  if(Inode->next_inode) {
    strcpy(status,"FREE");
  }
  else {
    strcpy(status,"USED");
  }
  printf("Inode %d is %s, the file size is %d\n",index, status, Inode->size);
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
    inode* inode_head = (inode*)(buffer + inode_start);
    inode* cur = inode_head;
    for(int i = 0; i < inode_num; i++) {
      print_inode(cur,i,1);
      cur = next_inode(cur);
  }
}