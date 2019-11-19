#ifndef __FS_H__
#define __FS_H__
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FSSIZE 10000000
#define BLOCKSIZE 4000
// (FSSIZE - 1MB) / BLOCKSIZE
#define NUMBLOCKS 2250

unsigned char* fs;

struct superblock {
  int freeBlockListSize, numInodes, numBlocks;
};

// does this even need to exist?
short int freeBlockList[NUMBLOCKS];

struct inode {
  // if file, NULL
  char directoryName[256];
  // can be f or d
  char type;
  // num blocks used
  int size;
  // contains ids of relevant data blocks
  int blockRef[100];
};

void mapfs(int fd);
void unmapfs();
void formatfs();
void loadfs();
void lsfs();
void addfilefs(char* fname);
void removefilefs(char* fname);
void extractfilefs(char* fname);

#endif
