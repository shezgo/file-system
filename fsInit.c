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
#include "directory_entry.h"
#include "volume_control_block.h"


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
int isBitUsed(uint8_t bitmap[], int blockNumber) {
    int byteIndex = blockNumber / 8;
    int bitIndex = blockNumber % 8;
    return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	printf("Size of DE:%ld", sizeof(DirectoryEntry));
	//This error check guarantees that the vcb can fit in block 0.
    if (blockSize < sizeof(VolumeControlBlock)) {
        fprintf(stderr, "Error: Block size (%ld) is less than sizeof(VolumeControlBlock) (%ld)\n",
                blockSize, sizeof(VolumeControlBlock));
        exit(EXIT_FAILURE);
    }

	//Allocate the size
	int totalBytes = numberOfBlocks * blockSize;
	uint8_t *bitmap = (uint8_t *)malloc(totalBytes);
    if (bitmap == NULL) {
        fprintf(stderr, "Failed to allocate memory for bitmap\n");
        exit(EXIT_FAILURE);
    }

	//Initialize all bits in the bitmap to 0 (free)
	memset(bitmap, 0, totalBytes); 

	/*Set the bits that we know are going to be occupied. This includes the block taken by 
	Professor's partition table in block 0, VCB in block 1, as well as blocks needed 
	to store the free space map. Per "steps for milestone 
	1 pdf" do not free the bitmap buffer. Keep it in memory and LBAwrite whenever you 
	manipulate the buffer. */
	
	int mapNumBlocks = (totalBytes + blockSize - 1)/(8*blockSize);

	for(int i = 0; i <= mapNumBlocks ; i++){
		setBit(bitmap, i);
	}

	//This just writes the free space map to disk
	LBAwrite(bitmap, mapNumBlocks, 1);



	/* Next initialize and write the vcb to disk. Store vcb using malloc
	and pass to LBAwrite. Do we free this buffer? The only modifiable variable is free_blocks
	*/

    DirectoryEntry *root = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    if (!root) {
        fprintf(stderr, "Failed to allocate memory for DirectoryEntry\n");
        return EXIT_FAILURE;
    }

	    // Initialize the name and other fields
    strcpy(root->name, "/");
    root->LBAlocation = 0;

	// Declare a time_t variable to store the current time
    time_t currentTime = time(NULL);

    // Check if getting the time was successful
    if (currentTime == (time_t)-1) {
        fprintf(stderr, "Failed to get the current time\n");
        return 1;
    }
    root->timeCreation = currentTime;
    root->lastAccessed = currentTime;
    root->lastModified = currentTime;
    root->isDirectory = 1;
	root->size = sizeof(root);

	// Initialize the array of directory
    for (int i = 0; i < root->numDE; ++i) {
        root->de[i] = NULL; // Initialize pointers to NULL
    }

	// Optionally, allocate and initialize entries for "." and ".."
    root->de[0] = root; // "." points to itself
    root->de[1] = root; // ".." also points to itself for the root directory

	//Assign root to a block
	//LBAwrite root to that block
	//Calculate number of blocks needed for root
	//**PICKUP LBAwrite(root, )
	//update the bitmap to occupy that block
	//Store root block location in vcb
	//Free root

	//First initialize the vcb
	VolumeControlBlock *vcb;

	vcb = (VolumeControlBlock *)malloc(sizeof(VolumeControlBlock));
	    if (vcb == NULL) {
        fprintf(stderr, "Failed to allocate memory for VCB\n");
        exit(EXIT_FAILURE);
    }

	vcb->block_size = blockSize;
	vcb->total_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks - mapNumBlocks - 1;//need to edit once root directory set
	vcb->signature = 0x1A; //8 bit hex number, max value 255 or 0xFF
	//vcb->root_directory_block = ?
	vcb->fsmap_start_block = 1;
	vcb->fsmap_end_block = mapNumBlocks + 1;
	//Next write vcb to disk at block 1
	LBAwrite(vcb, 1, 0);
	free(vcb);

//block_size/sizeOf(DirectoryEntry)

	return 0;
	}


	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}