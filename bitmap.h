/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
*
* File:: bitmap.h
*
* Description:: 
*	This is used to interact with the free space bitmap
*	
*	
*
**************************************************************/
#ifndef BITMAP_H
#define BITMAP_H

#include "fsLow.h"
#include "volume_control_block.h"
#include <stdio.h>
#include <stdlib.h>

extern VolumeControlBlock *vcb; // Global definition, always kept in memory 

typedef struct Bitmap
{
    uint8_t *bitmap;
    uint32_t mapNumBlocks; // number of blocks used to store the bitmap
    uint32_t fsNumBlocks;  // number of blocks in entire Volume
    uint32_t bitmapSize;   // size of the bitmap that *bitmap points to
} Bitmap;


// Function to set a bit (mark block as used). Returns 1 if success, -1 if failure.
int setBit(Bitmap *bm, int blockNumber);
// Function to clear a bit (mark block as free). Returns 1 if success, -1 if failure.
int clearBit(Bitmap *bm, int blockNumber);
// Function to check if a block is 1 (used). Returns 1 if block (bit) is being used.
int isBitUsed(Bitmap *bm, int blockNumber);
// Depending on requested number of blocks from user, return a starting block index to be used.
// Set all corresponding bits allocated to being used.
int fsAlloc(Bitmap *bm, int req);
// Set the corresponding bits in the bitmap to being free. Return 1 if successful, -1 if failed
int fsRelease(Bitmap* bm, int startBlock, int count);
//Initialize the bitmap
Bitmap *initBitmap(int fsNumBlocks, int blockSize, uint8_t * bm_bitmap);
//Write the bitmap to memory
uint8_t *loadBMtoMem(int blockSize);
//LBAwrite the freespace bitmap
int mapToDisk(Bitmap *bm);




#endif