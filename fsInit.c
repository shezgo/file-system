/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
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
#include "directory.c"
#include "volume_control_block.h"

uint8_t magicNumber;
VolumeControlBlock *vcb;
int mapNumBlocks; //used to store number of blocks needed for bitmap for easy LBA write
int totalBytes; //used to check bitmap for free space

void mapToDisk(uint8_t *bitmap){
	LBAwrite(bitmap, mapNumBlocks, 1);
	return;
}

// Function to set a bit (mark block as used)
void setBit(uint8_t *bitmap, int blockNumber)
{
	int byteIndex = blockNumber / 8;
	int bitIndex = blockNumber % 8;
	bitmap[byteIndex] |= (1 << bitIndex);
	return;
}

// Function to clear a bit (mark block as free)
void clearBit(uint8_t *bitmap, int blockNumber)
{
	int byteIndex = blockNumber / 8;
	int bitIndex = blockNumber % 8;
	bitmap[byteIndex] &= ~(1 << bitIndex);
	return;
}

// Function to check if a block is 1 (used). Returns 1 if block (bit) is being used.
int isBitUsed(uint8_t *bitmap, int blockNumber)
{
	int byteIndex = blockNumber / 8;
	int bitIndex = blockNumber % 8;
	return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

uint8_t firstFreeBit(uint8_t *bitmap)
{
	    for (int byteIndex = 0; byteIndex < totalBytes; byteIndex++) {
        // If the byte is not all 1s, there's a free bit in it
        if (bitmap[byteIndex] != 0xFF) {
            for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
                if ((bitmap[byteIndex] & (1 << bitIndex)) == 0) {
                    return (byteIndex * 8 + bitIndex);
                }
            }
        }
    }
	//If there are no free bits
    return -1;
}

//Initialize the root directory with the . and .. directory entries inside of it, then
//return the LBA block root is written to.
uint8_t initRoot(uint8_t *bitmap){
    Directory *root = (Directory *)malloc(sizeof(Directory));
    if (root == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for root directory.\n");
        return -1;
    }

    initDirectory(root);


    uint8_t rootBlock = firstFreeBit(bitmap);
    if (rootBlock == -1)
    {
        fprintf(stderr, "Error: No free block available for the root directory.\n");
        free(root); // Free the allocated memory before returning
        return -1;
    }

    // Allocate and initialize the "." directory entry
    DirectoryEntry *dot = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    if (dot == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for '.' directory entry.\n");
        free(root);
        return -1;
    }
	strncpy(dot->name, ".", NAME);
	dot->timeCreation = time(NULL);
	dot->lastAccessed = dot->timeCreation;
	dot->lastModified = dot->timeCreation;
	dot->isDirectory = 1; // Set to 1 because it points to self, a directory.
	dot->LBAlocation = rootBlock;
	dot->size = sizeof(DirectoryEntry);
	addDirectoryEntry(root, dot);

	// Allocate and initialize the ".." directory entry
    DirectoryEntry *dotdot = (DirectoryEntry *)malloc(sizeof(DirectoryEntry));
    if (dotdot == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory for '..' directory entry.\n");
        free(dot);
        free(root);
        return -1;
    }
    strncpy(dotdot->name, "..", NAME);
    dotdot->timeCreation = time(NULL);
    dotdot->lastAccessed = dotdot->timeCreation;
    dotdot->lastModified = dotdot->timeCreation;
    dotdot->isDirectory = 1; // Set to 1 because it points to self, a directory.
    dotdot->LBAlocation = rootBlock;
    dotdot->size = sizeof(DirectoryEntry);
    addDirectoryEntry(root, dotdot);

	printf("From fsInit: size of root:%ld\n", sizeof(root));
	// Write the root directory to disk
    LBAwrite(root, 1, rootBlock);
	setBit(bitmap, rootBlock);

    // Free the allocated memory
    free(dotdot);
    free(dot);
    free(root);

	return rootBlock;
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	vcb = (VolumeControlBlock *)malloc(sizeof(VolumeControlBlock));
	if (vcb == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for VCB\n");
		exit(EXIT_FAILURE);
	}
	// Check if magicNumber matches sig first before initializing file system/VCB
	if (vcb->signature == 0x1A)
	{
		fprintf(stderr, "Volume has already been initialized.");
		return 1;
	}

	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, 
	blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	printf("From fsInit: Size of DE:%ld\n", sizeof(DirectoryEntry));
	// This error check guarantees that the vcb can fit in block 0.
	if (blockSize < sizeof(VolumeControlBlock))
	{
		fprintf(stderr, "Error: Block size (%ld) is less than sizeof(VolumeControlBlock) (%ld)\n",
				blockSize, sizeof(VolumeControlBlock));
		exit(EXIT_FAILURE);
	}

	// Allocate the size
	totalBytes = numberOfBlocks * blockSize;
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

	// LBAwrite the bitmap at correct locations
	mapToDisk(bitmap);

	//Initialize the root directory and LBAwrite it to disk.
	uint8_t rootBlock = initRoot(bitmap);
	printf("From fsInit: mapNumBlocks is at %d\n", mapNumBlocks);
	printf("From fsInit: rootBlock is at %d\n", rootBlock);

	vcb->block_size = blockSize;
	vcb->total_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks - mapNumBlocks - 2; // subtract blocks for bitmap, vcb, root
	vcb->signature = 0x1A; // This is an arbitrary number to check if already initialized
	vcb->root_directory_block = rootBlock;
	vcb->fsmap_start_block = 1;
	vcb->fsmap_end_block = mapNumBlocks + 1;
	LBAwrite(vcb, 1, 0);
	free(vcb);



	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}