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

uint8_t magicNumber;
VolumeControlBlock *vcb;
int mapNumBlocks; //used to store number of blocks needed for bitmap for easy LBA write

// Function to set a bit (mark block as used)
void setBit(uint8_t bitmap[], int blockNumber)
{
	int byteIndex = blockNumber / 8;
	int bitIndex = blockNumber % 8;
	bitmap[byteIndex] |= (1 << bitIndex);
	return;
}

// Function to clear a bit (mark block as free)
void clearBit(uint8_t bitmap[], int blockNumber)
{
	int byteIndex = blockNumber / 8;
	int bitIndex = blockNumber % 8;
	bitmap[byteIndex] &= ~(1 << bitIndex);
	return;
}

// Function to check if a block is 1 (used). Returns 1 if block (bit) is being used.
int isBitUsed(uint8_t bitmap[], int blockNumber)
{
	int byteIndex = blockNumber / 8;
	int bitIndex = blockNumber % 8;
	return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

void mapToDisk(uint8_t bitmap[]){
	LBAwrite(bitmap, mapNumBlocks, 1);
	return;
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{

	// Check if magicNumber matches sig first before initializing file system/VCB
	if (vcb->signature == 0x1A)
	{
		printf("Volume has already been initialized.");
		return 1;
	}

	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, 
	blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	printf("Size of DE:%ld", sizeof(DirectoryEntry));
	// This error check guarantees that the vcb can fit in block 0.
	if (blockSize < sizeof(VolumeControlBlock))
	{
		fprintf(stderr, "Error: Block size (%ld) is less than sizeof(VolumeControlBlock) (%ld)\n",
				blockSize, sizeof(VolumeControlBlock));
		exit(EXIT_FAILURE);
	}

	// Allocate the size
	int totalBytes = numberOfBlocks * blockSize;
	uint8_t *bitmap = (uint8_t *)malloc(totalBytes);
	if (bitmap == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for bitmap\n");
		exit(EXIT_FAILURE);
	}

	// Initialize all bits in the bitmap to 0 (free)
	for(int i = 0; i < totalBytes; i ++){
		bitmap[i] = 0;
	}

	/*Set the bits that we know are going to be occupied. This includes the VCB in logical block 0,
	but physical block 0 contains Professor's partition table. Also set blocks needed
	to store the free space map. 
	Per "steps for milestone 1 pdf" do not free the bitmap buffer. Keep it in memory and 
	LBAwrite whenever you manipulate the buffer. */

	mapNumBlocks = (totalBytes + blockSize - 1) / (8 * blockSize);

	for (int i = 0; i <= mapNumBlocks; i++)
	{
		setBit(bitmap, i);
	}

	// This just writes the free space map to disk at correct locations
	mapToDisk(bitmap);

	/* Next initialize and write the vcb to disk. Store vcb using malloc
	and pass to LBAwrite. Do we free this buffer? The only modifiable variable is free_blocks
	*/


	// Assign root to a block
	// LBAwrite root to that block
	//**PICKUP create this function:Calculate number of blocks needed for root.
	// Needs to call a function that
	// asks the free space system for 6 blocks. It should return to you the starting
	// block number for those 6 blocks.
	//**PICKUP LBAwrite(root, )
	// update the bitmap to occupy that block
	// Store root block location in vcb
	// Free root

	// First initialize the vcb

	vcb = (VolumeControlBlock *)malloc(sizeof(VolumeControlBlock));
	if (vcb == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for VCB\n");
		exit(EXIT_FAILURE);
	}

	vcb->block_size = blockSize;
	vcb->total_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks - mapNumBlocks - 1; // need to edit once root directory set
	vcb->signature = 0x1A;								  // 8 bit hex number, max value 255 or 0xFF
	// vcb->root_directory_block = ?
	vcb->fsmap_start_block = 1;
	vcb->fsmap_end_block = mapNumBlocks + 1;
	// Next write vcb to disk at block 1
	LBAwrite(vcb, 1, 0);
	free(vcb);

	// block_size/sizeOf(DirectoryEntry)

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}