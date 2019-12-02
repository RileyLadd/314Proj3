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

  // format superblock
  struct superblock* sb = fs;
  sb->freeBlockListSize = NUMBLOCKS;
  sb->numInodes = NUM_INODES;
  sb->numBlocks = NUMBLOCKS;

  // format freeblockslist
  struct freeblockslist* fbl = fs + sizeof(struct superblock);
  for (int i = 0; i < NUMBLOCKS; i++) {
    fbl->freeBlocks[i] = 0;
  }
  
  // format inodes
  struct inode* inodes = fs + sizeof(struct superblock) + sizeof(struct freeblockslist);
  for (int i = 0; i < NUM_INODES; i++) {
    inodes[i].inuse = 0;
    inodes[i].type = -1;
    inodes[i].size = -1;
    for (int j = 0; j < 100; j++) {
      inodes[i].blockRef[j] = -1;
    }
  }

  // this step is probably meaningless for now
  // format blocks
  struct block* blocks = fs + sizeof(struct superblock) + sizeof(struct freeblockslist) + sizeof(struct inode)*NUM_INODES;

  // create root directory
  addfilefs("root");
  
}


void loadfs(){

  /* How to use fs for integers, as example.
     irrelevant to have here. just put it here
     for safe keeping

  int* intarray = fs;

  intarray[0] = 9099;
  intarray[100] = 934;
  */
  
  struct superblock* sb = fs;
  loadfbl = sb->freeBlockListSize;
  loadinode = sb->numInodes;
  loadblock = sb->numBlocks;
  
}


void lsfs(){
  
}

void addfilefs(char* fname){
  FILE * myfile = fopen(fname, "r");
  if(myfile == NULL) {
	printf("ERROR: Could not open %s for reading. Exiting.\n", fname);
	exit(1);
  }
  


  fclose(myfile);
}


void removefilefs(char* fname){

}


void extractfilefs(char* fname){

}
