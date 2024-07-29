#ifndef FSINIT_H
#define FSINIT_H


#include "directory_entry.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fsLow.h"
#include "mfs.h"

#include "volume_control_block.h"
#include "bitmap.h"

#define MIN_ENTRIES 15
#define CWD_SIZE 256


extern Bitmap * bitmap;
extern DE *rootGlobal; // Global declaration of the root directory
extern DE *cwdGlobal;  // Global declaration of the current working directory
extern char *cwdName; //Global array used to track cwd path string
extern VolumeControlBlock *vcb; // Global definition, always kept in memory 
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize);
void exitFileSystem();

#endif // FSINIT_H