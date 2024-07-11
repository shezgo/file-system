/**************************************************************
* Class::  CSC-415-0# Spring 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fsLow.h"
#include "mfs.h"
#include "directory_entry.c"
#include "volume_control_block.c"


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//This error check guarantees that the vcb can fit in block 0.
    if (blockSize < sizeof(VolumeControlBlock)) {
        fprintf(stderr, "Error: Block size (%ld) is less than sizeof(VolumeControlBlock) (%ld)\n",
                blockSize, sizeof(VolumeControlBlock));
        exit(EXIT_FAILURE);
    }

	//Allocate the size dynamically.
	int totalBytes = numberOfBlocks * blockSize;
	uint8_t bitmap[totalBytes];

	//Initialize all bits in the bitmap to 0 (free)
	memset(bitmap, 0, totalBytes); 

	/*Set the bits that we know are going to be occupied. This includes the block taken by 
	Professor's partition table in block 0, VCB in block 1, as well as blocks needed 
	to store the free space map. Per "steps for milestone 
	1 pdf" do not free the bitmap buffer. Keep it in memory and LBAwrite whenever you 
	manipulate the buffer. */
	
	int mapNumBlocks = (totalBytes + blockSize - 1)/(8*blockSize);

	for(int i = 0; i <= mapNumBlocks + 1; i++){
		setBit(bitmap, i);
	}

	LBAwrite(bitmap, mapNumBlocks, 2);



	/* Next initialize and write the vcb to disk. Store vcb using malloc
	and pass to LBAwrite. Do we free this buffer? The only modifiable variable is free_blocks
	*/
	
	//First initialize the vcb
	VolumeControlBlock *vcb;

	vcb = (VolumeControlBlock *)malloc(sizeof(VolumeControlBlock));
	    if (vcb == NULL) {
        fprintf(stderr, "Failed to allocate memory for VCB\n");
        exit(EXIT_FAILURE);
    }

	vcb->block_size = blockSize;
	vcb->total_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks - mapNumBlocks - 1;
	vcb->signature = 0x1A; //8 bit hex number, max value 255 or 0xFF
	//vcb->root_directory_block = ?
	vcb->fsmap_start_block = 1;
	vcb->fsmap_end_block = mapNumBlocks;
	//Next write vcb to disk at block 1
	LBAwrite(vcb, 1, 1);


	return 0;
	}

//Function to set a bit (mark block as used)
void setBit(uint8_t bitmap[], int blockNumber) {
    int byteIndex = blockNumber / 8;
    int bitIndex = blockNumber % 8;
    bitmap[byteIndex] |= (1 << bitIndex);
}
	
//Function to clear a bit (mark block as free)
void clearBit(uint8_t bitmap[], int blockNumber) {
    int byteIndex = blockNumber / 8;
    int bitIndex = blockNumber % 8;
    bitmap[byteIndex] &= ~(1 << bitIndex);
}

//Function to check if a block is 1 (used). Returns 1 if block (bit) is being used.
int isBlockUsed(uint8_t bitmap[], int blockNumber) {
    int byteIndex = blockNumber / 8;
    int bitIndex = blockNumber % 8;
    return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
}
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}