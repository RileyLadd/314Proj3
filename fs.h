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

typedef struct superblock {
  int freeBlockListSize, numInodes, numBlocks;
} superblock;

// does this even need to exist?
//short int freeBlockList[NUMBLOCKS];

typedef struct inode {
  // if file, NULL
  // char directoryName[256];
  int inuse;
  // can be f (1) or d (0)
  int type;
  // num bytes of file
  int size;
  // contains ids of relevant data blocks
  int blockRef[100];
} inode;

typedef struct freeblockslist {
  int freeBlocks[NUMBLOCKS];
} freeblockslist;

typedef struct block {
  char data[BLOCKSIZE];
} block;

typedef struct directoryEntry {
  char * name;
  //char * name2;
  char * files[100]; // which can include directories
  int inuse; //1 is inuse, 0 is empty/invalid
} directoryEntry;

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
void addfilefs(const char* fname);
void removefilefs(char* fname);
void extractfilefs(char* fname);

int nextInode();
int nextBlock();

#endif
