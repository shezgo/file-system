/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
 *
 * File:: fsInit.h
 *
 * Description:: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/
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
#define MAX_DIR_NAME 100

extern Bitmap *bm;      //Global declaration of the freespace bitmap
extern DE *rootGlobal; // Global declaration of the root directory
extern DE *cwdGlobal;  // Global declaration of the current working directory
extern char *cwdName; //Global array used to track cwd path string
extern VolumeControlBlock *vcb; // Global definition, always kept in memory 
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize);
void exitFileSystem();
VolumeControlBlock* loadVCBtoMem(uint64_t blockSize);

#endif // FSINIT_HS