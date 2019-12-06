
#ifndef __FS_H__
#define __FS_H__
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define FSSIZE 10000000
#define BLOCKSIZE 512
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


typedef struct freeblockslist {
  short freeBlocks[NUMBLOCKS];
} freeblockslist;

typedef struct inode {
  short inuse;
  // can be f (1) or d (0)
  short type;
  // num bytes of file
  // all directories will get 50 blocks for simplicity
  unsigned int size;
  // contains ids of relevant data blocks
  short blockRef[100];
} inode;

typedef struct block {
  char data[BLOCKSIZE];
} block;

typedef struct directoryEntry {
  short inuse;
  char name[256];
  short inodeLoc;
} directoryEntry;

void mapfs(int fd);
void unmapfs();
void formatfs();
void loadfs();
void lsfs();
void addfilefs(const char* fname);
void removefilefs(char* fname);
void extractfilefs(char* fname);

int nextInode();
void fileAdder(char* parent, char* filename, int filesize, int isDir);
int nextBlock(int isDir);

#endif
