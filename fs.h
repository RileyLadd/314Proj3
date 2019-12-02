#ifndef __FS_H__
#define __FS_H__
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FSSIZE 10000000
#define BLOCKSIZE 512
// (FSSIZE - 1MB) / BLOCKSIZE
#define NUMBLOCKS 10000
#define NUM_INODES 100

unsigned char* fs;

// values taken from existing fs being loaded
// originally set these 3 equal to 0; Riley changed
int loadfbl;
int loadinode;
int loadblock;

struct superblock {
  int freeBlockListSize, numInodes, numBlocks;
};

// does this even need to exist?
//short int freeBlockList[NUMBLOCKS];

struct inode {
  // if file, NULL
  // char directoryName[256];
  int inuse;
  // can be f (1) or d (0)
  int type;
  // num blocks used
  int size;
  // contains ids of relevant data blocks
  int blockRef[100];
};

struct freeblockslist {
  int freeBlocks[NUMBLOCKS];
};

struct block {
  char data[BLOCKSIZE];
};

struct directoryEntry {
  char name[256];
};

/* example
   struct inode{
     int inuse;
     int type;
     int size;
     //initialize to -1
     int blocks[100];
   };

 */

void mapfs(int fd);
void unmapfs();
void formatfs();
void loadfs();
void lsfs();
void addfilefs(char* fname);
void removefilefs(char* fname);
void extractfilefs(char* fname);

#endif
