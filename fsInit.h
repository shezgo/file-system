#ifndef FSINIT_H
#define FSINIT_H



#include "volume_control_block.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fsLow.h"
#include "mfs.h"
#include "directory_entry.h"
#include "bitmap.h"

#define MIN_ENTRIES 50

extern DE *rootGlobal; // Global declaration of the root directory
extern DE *cwdGlobal;  // Global declaration of the current working directory
extern VolumeControlBlock *vcb; // Global definition, always kept in memory 
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize);
void exitFileSystem();

#endif // FSINIT_H