#include "fs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


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
  addfilefs("root", 0);
  
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

void addfilefs(const char* fname, int fileSize){
  

  if(strcmp(fname, "root") == 0 && nextInode() == 0) { //we need to add the root (home) directory
    int i = 0, currblock = 0;
    int totalblocksused = 0;
    char* currmem = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
    freeblockslist* fbl = fs + sizeof(superblock);
    //fbl->freeBlocks[0] = 1;
    inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
    in->inuse = 1; //indicates inode is being used
    in->type = 0; //0 indicates this is a directory
    in->size = 25860; //each name 256bytes large, 101 names in a directory, int 4bytes large

    // file name into inode
    strcpy(in->fileName, fname);
    
    // directories need 51 blocks
    for (int i = 0; i < 51; i++) {
      in->blockRef[i] = i; // data block being used is at index i
      fbl->freeBlocks[i] = 1;
      
    }
    //printf("inode inuse val at index 100 is %d\n", fbl->freeBlocks[100]);
    /*
    //printf("ree\n");
    // Now we need to format the root directory shtuff/data block
    directoryEntry* dentry = (directoryEntry *) malloc(sizeof(directoryEntry));
    (*dentry).name = (char *) malloc(256);
    //(*dentry).name2 = (char *) malloc(2569999999);
    for(i = 0; i < 100; i++) {
      (*dentry).files[i] = (char *) malloc(256);
    }
    strcpy( (*dentry).name, fname);
    //printf("ree2\n");
    dentry->inuse = 0; //nothing in the root directory yet
    //printf("size of directory entry is %d\n", sizeof(*dentry));

    totalblocksused = (256 + 100*256) / 512 + 1;
    printf("total blocks used: %d\n", totalblocksused);
    //in->size = totalblocksused;
    for(i = 0; i < totalblocksused; i++) {
      currblock = nextBlock();
      in->blockRef[i] = currblock;
      fbl->freeBlocks[currblock] = 1;
    }
    //block* blockthing = fs + stuff;
    char * currmem = fs;
    for( i = 0; i < totalblocksused; i++) {
      //fwrite( (*dentry).name, 256, 1, fs );
      //fwrite( (*dentry)
      if(i == 0) {
	currmem = fname;
	currmem += 256;
      }
      currmem = (*dentry).files[i];
      currmem += 256;
    }
    */

    // put root name into first block
    strcpy(currmem, fname);
    
  } // end of root

  else {
    // printf("ree\n");
    int fd = -1;
    struct stat stats;
    int filesize = 0;
    
    int currBlock = 0;
    char* lastpath;
    char* path = strtok(fname, "/");
    int pathExists;
    int pathLength = 1;
    int blocksNeeded;
    //printf("filesize is %d\n", fileSize);
    int freeInode = nextInode();
    int freeBlock = nextBlock();
    int i = 1;
    freeblockslist* fbl = fs + sizeof(superblock);
    //fbl->freeBlocks[0] = 1;
    inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
    inode* freein = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*freeInode;
    char* currmem = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
    // printf("ree\n");
    // divide path
    //printf("%s\n", fname);
    //strcpy(path[0], strtok(fname, '/'));
    /*
    for (int i = 1; i < 10; i++) {
      path[i] = strtok(NULL, '/');
    }
    */
    printf("ree\n");
    /*
    while ((strcpy(path[i], strtok(NULL, '/'))) != NULL) {
      i++;
      printf("%d\n", i);
      pathLength++;
    }
    */
    while(path != NULL) {
      printf("%s\n", path);
      lastpath = path;
      path = strtok(NULL, "/");
    }
    if(path == NULL) {
      //strcpy(path, lastpath);
    }
    fd = open(lastpath, O_RDONLY);
    fstat(fd, &stats);
    filesize = stats.st_size;
    blocksNeeded = filesize/BLOCKSIZE + 1;
    
    FILE * myfile = fopen(lastpath, "r");
    if(myfile == NULL) {
      printf("path is %s\n", lastpath);
      printf("ERROR: Could not open %s for reading. Exiting.\n", fname);
      exit(1);
    } else {
      printf("opened %s\n", lastpath);
    }

    
    // check if directory exists
    /*
    while (in->inuse != 0) {
      if (strcmp(in->fileName, fname) >= 0;) { pathExists = 1; }
      in += sizeof(inode);
    }

    // create subdirectories if not exist
    if (pathExists == 0) {
      for (i = 0; i < pathLe
    }
    */
    
    // create inode for file
    strcpy(in->fileName, fname);
    freein->inuse = 1;
    freein->type = 1;
    freein->size = fileSize;
    printf("%d blocks needed\n", blocksNeeded);
    for (i = 0; i < blocksNeeded; i++) {
      currmem += (sizeof(block)*(freeBlock - currBlock));
      currBlock = freeBlock;
      freein->blockRef[i] = freeBlock;
      fbl->freeBlocks[freeBlock] = 1;
      fwrite(currmem, 512, 1, myfile);
      freeBlock = nextBlock();
    }

    // write file data to blocks

    fclose(myfile);
  }
  /*
  FILE * myfile = fopen(fname, "r");
  if(myfile == NULL) {
	printf("ERROR: Could not open %s for reading. Exiting.\n", fname);
	exit(1);
  }
  */


  //fclose(myfile);
}


void removefilefs(char* fname){

}


void extractfilefs(char* fname){

}

// EXTRA FUNCTIONS WE DEFINED

int nextInode() {
  int myinode = 0;
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  while(in->inuse != 0) {
    in = in + sizeof(inode);
    myinode++;
    if (myinode > 100) { return -1; }
  }
  
  
  return myinode;
}
int nextBlock() {
  int myblock = 0;
  freeblockslist* fbl = fs + sizeof(superblock);
  //NUMBLOCKS = 10000
  //printf("%d\n", fbl->freeBlocks[0]);
  while(fbl->freeBlocks[myblock] != 0 && myblock < NUMBLOCKS) {
    myblock++;
  }
  if(myblock == NUMBLOCKS) {
    printf("ERROR: There are no free blocks. File system is full. Exiting.\n");
    exit(1);
  }

  return myblock;
  
}
