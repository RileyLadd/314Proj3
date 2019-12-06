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
    // reserve first 50 blocks for root
    if (i < 50) { fbl->freeBlocks[i] = 1; }
    else        { fbl->freeBlocks[i] = 0; }
  }
  
  // Format inodes
  struct inode* inodes = fs + sizeof(struct superblock) + sizeof(struct freeblockslist);

  // reserve inode 1 for root
  inodes->inuse = 1;
  inodes->type = 0;
  inodes->size = BLOCKSIZE*50;
  //printf("%d %d %d\n", inodes->inuse, inodes->type, inodes->size);
  for (int j = 0; j < 50; j++) {
    inodes->blockRef[j] = j;
  }

  inodes += sizeof(struct inode);
  for (int i = 1; i < NUM_INODES; i++) {
   
    inodes->inuse = 0;
    inodes->type = -1;
    inodes->size = -1;
    //printf("%d %d %d\n", inodes->inuse, inodes->type, inodes->size);
    for (int j = 0; j < 100; j++) {
      inodes->blockRef[j] = -1;
    }
    inodes += sizeof(struct inode);

  }

  printf("AM ROOT? %d\n", inodes->blockRef[5]);

  /*
  printf("%d\n", inodes[42].blockRef[35]);
  inodes = fs + sizeof(struct superblock) + sizeof(struct freeblockslist);
  printf("%d\n", inodes->blockRef[67]);
  inodes += sizeof(struct inode)*5;
  printf("%d\n", inodes->blockRef[42]);
  */
}


void loadfs(){
  
  struct superblock* sb = fs;
  loadfbl = sb->freeBlockListSize;
  loadinode = sb->numInodes;
  loadblock = sb->numBlocks;
  
} // end loadfs


void lsfs(){
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  printf("File System Contents:\n");
  while(in->inuse != 0) {
   
    printf("%s\n", in->blockRef[0]);

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
} // end lsfs

void addfilefs(const char* fname){
  
  int fd = -1;
  struct stat stats;
  int filesize = 0;
  
  // Fields used for duplicate checking
  //char * dupcheck;
  //int dupflag; // 0 for no dup, 1 for dup
      
  //int currBlock = 0;
  char* lastlastpath;
  char* lastpath = NULL;
  char* path = strtok(fname, "/");
  char fullpath[256] = ".";
  char fullpath2[256] = ".";
  int pathExists;
  int pathLength = 1;
  int blocksNeeded;
  int isDir;

  
  int freeInode;
  
  //int freeBlock = nextBlock();
  //int i = 1;
  //freeblockslist* fbl = fs + sizeof(superblock);
  
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  inode* freein;
    
  //char* currmem = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
  //directoryEntry* currdir = currmem;
  
  //dupcheck = currmem + BLOCKSIZE*51;
  //dupcheck = currmem;
  
  freeInode = 0;
  freein = fs + sizeof(superblock) + sizeof(freeblockslist);
  while(freein->inuse == 1) {
    freein += sizeof(inode);
    freeInode++;
  }
  
  while(path != NULL) {
    lastlastpath = lastpath;
    lastpath = path;
    strcat(fullpath, "/");
    strcat(fullpath, lastpath);
    path = strtok(NULL, "/");
  }

  // stick file name and inode id into directory
  //strcpy(currdir->name, fullpath);
  //currdir->inodeLoc = freeInode;
  
  fd = open(fullpath, O_RDONLY);
  fstat(fd, &stats);
  filesize = stats.st_size;
  blocksNeeded = filesize/BLOCKSIZE + 1;
  
    
  if(filesize > 5120000) {
    printf("ERROR: File is too large to fit in file system.\n");
      exit(1);
  }

  lastlastpath = NULL;
  path = NULL;
  lastpath = NULL;
  path = strtok(fullpath, "/");
  path = strtok(NULL, "/");
  printf("fname: %s\n", fullpath);

  // add files/directories
  while(path != NULL) {
    lastlastpath = lastpath;
    lastpath = path;
    strcat(fullpath2, "/");
    strcat(fullpath2, lastpath);
    printf("AM BOI %s\n", fullpath2);
    fd = open(fullpath2, O_RDONLY);
    fstat(fd, &stats);
    isDir = !S_ISDIR(stats.st_mode)? 0 : 1;
    fileAdder(lastlastpath, fullpath2, filesize, isDir);
    printf("left fileAdder\n");
    printf("%s\n", path);
    path = strtok(NULL, "/");
    printf("%s\n", path);
  }
  /*
    FILE * myfile = fopen(fullpath, "r");
    if(myfile == NULL) {
  
    printf("ERROR: Could not open %s for reading. Exiting.\n", fname);
    exit(1);
    }
  
    // fill out inode for file
    freein->inuse = 1;
    freein->type = 1;
    freein->size = filesize;
    
    // write directory data to block
  
  
    // write file data to block
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
  */
  
  //fclose(myfile);
  
} // end addfilefs


void removefilefs(char* fname){
  /*  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);

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
*/
} // ends removefilefs


void extractfilefs(char* fname){
  /*  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
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
    read = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100 + BLOCKSIZE*(in->blockRef[j]);
    for(i = 0; i < 512; i++) {
      printf("%c", read[i]);
      sizecounter++;
      if(sizecounter == in->size) {
	return;
      }
    }
    j++;
  }
*/
} //end extractfilefs


// EXTRA FUNCTIONS WE DEFINED
/*
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
} // end nextInode
*/

void fileAdder(char* parent, char* filename, int filesize, int isDir) {
  int infs = 0;
  int numDirBlocks = 50;
  int currDirBlock = 0;
  int numDir = 98;
  int parentFound = 0;
  //int fd = -1;
  //struct stat stats;
  //int filesize = 0;
  
  // Fields used for duplicate checking
  char * dupcheck;
  int dupflag; // 0 for no dup, 1 for dup
      
  int currBlock = 0;
  //char* lastlastpath;
  //char* lastpath;
  //char* path = strtok(fname, "/");
  //char fullpath[256] = ".";
  //int pathExists;
  //int pathLength = 1;
  int blocksNeeded;
  //int isDir;

  int inCount = 0;
  
  int freeInode;
  int inuseFlag = 0;
  int freeBlock = nextBlock(isDir);
  int i = 1;
  int j = 0;
  freeblockslist* fbl = fs + sizeof(superblock);
  
  inode* in = fs + sizeof(superblock) + sizeof(freeblockslist);
  inode* freein;
  inode* basein = in;
    
  char* currmem = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(inode)*100;
  char* basemem = currmem;
  directoryEntry* currdir = currmem;

  //strcat(fullpath, filename);
  FILE * myfile = fopen(filename, "r");
  if(myfile == NULL) {
      
    printf("ERROR: Could not open %s for reading. Exiting.\n", filename);
    exit(1);
  }
  
  //dupcheck = currmem + BLOCKSIZE*51;
  dupcheck = currmem;
  freeInode = 1;
  freein = fs + sizeof(superblock) + sizeof(freeblockslist) + sizeof(struct inode);
  //printf("inode in use? %d\n", freein->inuse);
  while(freein->inuse == 1) {
    freein += sizeof(inode);
    freeInode++;
  }

  // check if file/directory is already in the fs
  //printf("AM ROOT? %d\n", in->blockRef[5]);

  inCount = 0;
  while (in->inuse == 1 && infs == 0 && inCount <= 100) {
    if (in->type == 0) {
      while (in->blockRef[j] != -1 && infs == 0) {
	currmem = basemem + BLOCKSIZE*in->blockRef[j];
	currdir = currmem;
	// default of 99 max directories in directories
	for ( int f = 0; f < 99; f++) {
	  // check if filename is in currdir
	  //printf(" using dir? %d", currdir->inuse);
	  if (printf(currdir->inuse) && currdir->inuse == 1) {
	    if (strcmp(currdir->name, filename) >= 0) {
	      //printf("%s %s\n", currdir->name, filename);
	      infs = 1;
	    }
	  }
	  currdir += sizeof(struct directoryEntry);
	}
	j++;
      }
    }
    
    in += sizeof(inode);
  }
  

  if (infs == 2) {
    printf("in fs\n");
      
    return; }
  else {
    if (infs) { printf("in fs\n"); }
    printf("not in fs\n");
    
    if (isDir) {
      printf("is dir\n");
      // check if directory is in root
      if (parent == NULL) {
	printf("in root\n");

	// set currdir to first block
	currdir = basemem;
	// search for first available directory entry
	while (currdir->inuse == 1) {
	  printf("dir taken by %s %d\n", currdir->name, currdir->inodeLoc);
	  currdir += sizeof(struct directoryEntry);
	}

	// fill out directoryEntry
	currdir->inuse = 1;
	strcpy(currdir->name, filename);
	currdir->inodeLoc = freeInode;

	// fill out inode
	freein->inuse = 1;
	freein->type = 0;
	freein->size = 50; //50 blocks
	freein->blockRef[nextBlock(isDir)]; //only need to find first block for dir
	
	// capture 50 contiguous blocks
	for (i = freein->blockRef[0]; i < 50; i++) {
	  fbl->freeBlocks[i] = 1;
	}
	
      } else {
	printf("in parent\n");

	
	currdir = basemem;
	in = basein;
	inCount = 0;

	// find parent directory
	while (in->inuse == 1 && !parentFound && inCount <= 100) {
	  if (in->type == 0) {
	    // go to first dir block
	    currdir = basemem + BLOCKSIZE*(in->blockRef[0]);
	    // look at each dir
	    for (currDirBlock = 0; currDirBlock < numDir; currDirBlock++) {
	      if (strcmp(currdir->name, parent) >= 0) {
		// parent found
		printf("parent found\n");
		parentFound = 1;
	      }
	      parentFound = 1;
	      currdir += sizeof(directoryEntry);
	    }
	  }
	  
	  in += sizeof(struct inode);
	  inCount++;
	}
      }
      
    } else {
      printf("is file\n");
      // fill out inode for file
      freein->inuse = 1;
      freein->type = 1;
      freein->size = filesize;
      
      // write file data to block
      for (i = 0; i < blocksNeeded; i++) {
	//currmem += (sizeof(block)*(freeBlock - currBlock));
	currmem = basemem + BLOCKSIZE*freeBlock;
	currBlock = freeBlock;
	freein->blockRef[i] = freeBlock;
	// capture block
	fbl->freeBlocks[freeBlock] = 1;
	for(int j = 0; j < 512; j++) {
	  currmem[j] = fgetc(myfile);
	}
	// Check if most recently written block is a Duplicate block
	for(int b = 51; b < NUMBLOCKS; b++) {
	  
	  dupflag = 1;
	  currBlock = b; // The existing block we're currently checking against
	  if(currBlock != freeBlock && fbl->freeBlocks[currBlock] == 1) { // Prevents checking block against itself and makes sure block is in use
	  
	    dupcheck = basemem + BLOCKSIZE*currBlock;
	    for( int j = 0; j < 512; j++ ) {
	      if(currmem[j] != dupcheck[j]) {
		
		dupflag = 0;
	      }
	      if( dupflag == 1 && j == 511 && (currmem[j] == dupcheck[j]) ) {
		freein->blockRef[i] = currBlock;
		fbl->freeBlocks[currBlock] = 1;
		// free dup block
		fbl->freeBlocks[freeBlock] = 0;
		
	      }
	    
	    }
	  } 
	}
	currBlock = freeBlock;
	freeBlock = nextBlock(isDir);
      }
    }
  }
  
  fclose(myfile);
  
  return;
}
  
int nextBlock(int isDir) {
  int myblock = 0;
  freeblockslist* fbl = fs + sizeof(superblock);
  // directories get the first 1000 blocks
  if (isDir) {
    while(fbl->freeBlocks[myblock] != 0 && myblock <= 999) {
      myblock++;
    }
  } else {
    myblock = 1000;
    while(fbl->freeBlocks[myblock] != 0 && myblock < NUMBLOCKS) {
      myblock++;
    }
  }
  if(myblock == NUMBLOCKS || myblock == 999) {
    printf("ERROR: There are no free blocks. File system is full. Exiting.\n");
    exit(1);
  }
  
  return myblock;
  
}
  
