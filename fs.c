#include "fs.h"

void mapfs(int fd){
  if ((fs = mmap(NULL, FSSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == NULL){
      perror("mmap failed");
      exit(EXIT_FAILURE);
  }
}


void unmapfs(){
  munmap(fs, FSSIZE);
}

void formatfs(){
  struct superblock sb = {NUMBLOCKS, NUMBLOCKS, NUMBLOCKS};
  int headSize = sizeof(struct superblock) + NUMBLOCKS + sizeof(struct inode);

  // following code probably won't work but idk
  
  // write superblock data to fs
  fs[0] = (char)sb.freeBlockListSize;
  fs[1] = (char)sb.numInodes;
  fs[2] = (char)sb.numBlocks;

  // write free blocks to fs
  for (int i = 4; i < NUMBLOCKS; i++) {
    fs[4] = '0';
  }

  // write root to fs
  
  
}


void loadfs(){

}


void lsfs(){
  
}

void addfilefs(char* fname){

}


void removefilefs(char* fname){

}


void extractfilefs(char* fname){

}
