#ifndef BITMAP_H
#define BITMAP_H

#include "fsLow.h"
#include "volume_control_block.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct Bitmap
{
    uint8_t *bitmap;
    uint32_t mapNumBlocks; // number of blocks used to store the bitmap
    uint32_t fsNumBlocks;  // number of blocks in entire Volume
    uint32_t bitmapSize;   // size of the bitmap that *bitmap points to
} Bitmap;


// Function to LBAwrite the freespace bitmap
void mapToDisk(Bitmap *bm);
// Function to set a bit (mark block as used)
int setBit(Bitmap *bm, int blockNumber);
// Function to clear a bit (mark block as free)
int clearBit(Bitmap *bm, int blockNumber);
// Function to check if a block is 1 (used). Returns 1 if block (bit) is being used.
int isBitUsed(Bitmap *bm, int blockNumber);
// Depending on requested number of blocks from user, send a starting block to be used.
int fsAlloc(Bitmap *bm, int req);
//To be implemented
int fsRelease(Bitmap* bm, int startBlock, int count);
//Initialize the bitmap
Bitmap *initBitmap(int fsNumBlocks, int blockSize);



#endif