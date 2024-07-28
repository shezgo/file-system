#ifndef FSINIT_H
#define FSINIT_H

#include <stdint.h>
#include "directory_entry.h"

extern DE *rootGlobal; // Global declaration of the root directory
extern DE *cwdGlobal;  // Global declaration of the current working directory
extern VolumeControlBlock *vcb; // Global definition, always kept in memory 
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize);
void exitFileSystem();

#endif // FSINIT_H