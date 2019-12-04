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

  // Format superblock
  struct superblock* sb = fs;
  sb->freeBlockListSize = NUMBLOCKS;
  sb->numInodes = NUM_INODES;
  sb->numBlocks = NUMBLOCKS;

  // Format freeblockslist
  struct freeblockslist* fbl = fs + sizeof(struct superblock);
  for (int i = 0; i < NUMBLOCKS; i++) {
    fbl->freeBlocks[i] = 0;
  }
  
  // Format inodes
  struct inode* inodes = fs + sizeof(struct superblock) + sizeof(struct freeblockslist);
  for (int i = 0; i < NUM_INODES; i++) {
   
    inodes[i].inuse = 0;
    inodes[i].type = -1;
    inodes[i].size = -1;
    for (int j = 0; j < 100; j++) {
      inodes[i].blockRef[j] = -1;
    }

  }

  // Format blocks
  struct block* blocks = fs + sizeof(struct superblock) + sizeof(struct freeblockslist) + sizeof(struct inode)*NUM_INODES;

  // Create root directory
  addfilefs("root", 0);
  
}


void loadfs(){
  
  struct superblock* sb = fs;
  loadfbl = sb->freeBlockListSize;
  loadinode = sb->numInodes;
  loadblock = sb->numBlocks;
  
}


void lsfs(){
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  printf("File System Contents:\n");
  while(in->inuse != 0) {
   
    printf("%s\n", in->fileName);

    /* // NOTE:
       // UNCOMMENT THIS SECTION TO VIEW THE DATA BLOCKS EACH INODE REFERENCES
       // THIS HELPS TO SEE THAT NO BLOCKS ARE DUPLICATED
    
    for(int i = 0; i < 100; i++) {
      printf("%d ", in->blockRef[i]);
    }
    printf("\n");
    */
    
    in += sizeof(inode);
  }
}

void addfilefs(const char* fname, int fileSize){
  

  if(strcmp(fname, "root") == 0 && nextInode() == 0) { // We need to add the root (home) directory
    int i = 0, currblock = 0;
    int totalblocksused = 0;
    char* currmem = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
    freeblockslist* fbl = fs + sizeof(superblock);
    inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
    in->inuse = 1;	// Indicates inode is being used
    in->type = 0; 	// 0 indicates this is a directory
    in->size = 25860; 	// Each name 256bytes large, 101 names in a directory, int 4bytes large

    // File name into inode
    strcpy(in->fileName, "/root");
    
    // Directories need 51 blocks
    for (int i = 0; i < 51; i++) {
      in->blockRef[i] = i; // Data block being used is at index i
      fbl->freeBlocks[i] = 1;
      
    }
    in += sizeof(inode);
    
  } // End of root creation

  else {
    
    int fd = -1;
    struct stat stats;
    int filesize = 0;

    // Fields used for duplicate checking
    char * dupcheck;
    int dupflag; // 0 for no dup, 1 for dup
      
    int currBlock = 0;
    char* lastpath;
    char* path = strtok(fname, "/");
    int pathExists;
    int pathLength = 1;
    int blocksNeeded;
    
    int freeInode = nextInode();
    
    if(strcmp(fname, "testFile") == 0) {
      freeInode = 2;
    }
    
    int freeBlock = nextBlock();
    int i = 1;
    freeblockslist* fbl = fs + sizeof(superblock);
    
    inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
    inode* freein;
    
    char* currmem = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
    dupcheck = currmem + BLOCKSIZE*51;
    freeInode = 0;
    freein = fs + sizeof(superblock) + sizeof(freeblockslist);
    while(freein->inuse == 1) {
      freein += sizeof(inode);
      freeInode++;
    }
    
    while(path != NULL) {
      
      lastpath = path;
      strcat(freein->fileName, "/");
      strcat(freein->fileName, lastpath);
      path = strtok(NULL, "/");
    }
    
    fd = open(lastpath, O_RDONLY);
    fstat(fd, &stats);
    filesize = stats.st_size;
    blocksNeeded = filesize/BLOCKSIZE + 1;

    if(filesize > 9000000) {
      printf("ERROR: File is too large to fit in file system.\n");
      exit(1);
    }
    
    FILE * myfile = fopen(lastpath, "r");
    if(myfile == NULL) {
      
      printf("ERROR: Could not open %s for reading. Exiting.\n", fname);
      exit(1);
    }
    
    freein->inuse = 1;
    freein->type = 1;
    freein->size = filesize;
    
    for (i = 0; i < blocksNeeded; i++) {
      currmem += (sizeof(block)*(freeBlock - currBlock));
      currBlock = freeBlock;
      freein->blockRef[i] = freeBlock;
      fbl->freeBlocks[freeBlock] = 1;
      for(int j = 0; j < 512; j++) {
	currmem[j] = fgetc(myfile);
      }
      // Check if most recently written block is a Duplicate block
      for(int b = 51; b < NUMBLOCKS; b++) {
	
	  dupflag = 1;
	  currBlock = b; // The existing block we're currently checking against
	  if(currBlock != freeBlock && fbl->freeBlocks[currBlock] == 1) { // Prevents checking block against itself and makes sure block is in use
	   
	    dupcheck = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100 + BLOCKSIZE*currBlock;
	    for( int j = 0; j < 512; j++ ) {
	      if(currmem[j] != dupcheck[j]) {
		
		dupflag = 0;
	      }
	      if( dupflag == 1 && j == 511 && (currmem[j] == dupcheck[j]) ) {
		freein->blockRef[i] = currBlock;
		fbl->freeBlocks[currBlock] = 1;
		fbl->freeBlocks[freeBlock] = 0;
		
	      }
	      
	    }
	  } 
      }
      
      
      currBlock = freeBlock;
      freeBlock = nextBlock();
    }

    fclose(myfile);
  }
}


void removefilefs(char* fname){
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);

  while(in->inuse == 1) {
    if(strcmp(fname, in->fileName) == 0) {
      break;
    }
    in += sizeof(inode);
  }
  if (in->inuse == 0) {
    printf("File does not exist\n");
    exit(1);
  }
  in->inuse = 0;
  
}


void extractfilefs(char* fname){
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  int i = 0, j = 0, sizecounter = 0;
  char * read = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
 
  while(in->inuse == 1) {
    if(strcmp(fname, in->fileName) == 0) {
      break;
    }
    in += sizeof(inode);
  }
  if(in->inuse == 0) {
    printf("Specified file does not exist in this file system. Exiting.\n");
    exit(1);
  }
  
  while(in->blockRef[j] > 0) {
    read = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100 + 512*(in->blockRef[j]);
    for(i = 0; i < 512; i++) {
      printf("%c", read[i]);
      sizecounter++;
      if(sizecounter == in->size) {
	return;
      }
    }
    j++;
  }
}

// EXTRA FUNCTIONS WE DEFINED
// Don't use nextInode() because it doesn't work for unknown reasons
int nextInode() {
  int myinode = 0;
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  while(in->inuse != 0) {
    in += sizeof(inode);
    myinode++;
    if (myinode >= 100) {
      printf("ERROR: Out of inodes! Exiting.\n");
      exit(1);
      return -1;
    }
  }
  
  
  return myinode;
}
int nextBlock() {
  int myblock = 0;
  freeblockslist* fbl = fs + sizeof(superblock);
  while(fbl->freeBlocks[myblock] != 0 && myblock < NUMBLOCKS) {
    myblock++;
  }
  if(myblock == NUMBLOCKS) {
    printf("ERROR: There are no free blocks. File system is full. Exiting.\n");
    exit(1);
  }

  return myblock;
  
}
