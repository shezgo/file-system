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
#include "fsInit.h"

uint8_t magicNumber;
int mapNumBlocks;		 // used to store number of blocks needed for bitmap for easy LBA write
int totalBytes;			 // used to check bitmap for free space
VolumeControlBlock *vcb = NULL; // Global definition, always kept in memory
DE *rootGlobal= NULL;			 // Global definition, always kept in memory
DE *cwdGlobal = NULL;			 // Global definition, always kept in memory
char *cwdName= NULL;			 // Global char* used to track cwd path string
Bitmap *bm = NULL;		 		//Global declaration of the freespace bitmap

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	cwdName = (char *)malloc(CWD_SIZE);
	if (cwdName == NULL)
	{
		fprintf(stderr, "cwdName memory allocation failed\n");
		return -1;
	}
	for(uint32_t i = 0; i < CWD_SIZE; i++)
	{
		cwdName[i] = '\0';
	}

		// This error check guarantees that the vcb can fit in block 0.
		if (blockSize < sizeof(VolumeControlBlock))
		{
			fprintf(stderr, "Error: Block size (%ld) is less than sizeof(VolumeControlBlock) (%ld)\n",
					blockSize, sizeof(VolumeControlBlock));
			exit(EXIT_FAILURE);
		}

	vcb = (VolumeControlBlock *)malloc(blockSize);
	if (vcb == NULL)
	{
		fprintf(stderr, "vcb Memory allocation failed\n");
		return -1;
	}

	// Initialize all bytes to 0 using a for loop
	for (uint32_t i = 0; i < blockSize; i++)
	{
		((char *)vcb)[i] = 0;
	}
	// Check if magicNumber matches sig first before initializing file system/VCB
	if (vcb->signature == 0x1A)
	{
		fprintf(stderr, "Volume has already been initialized.");
		return 1;
	}

	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks,
		   blockSize);

	printf("From fsInit: Size of DE:%ld\n", sizeof(DE));
	bm = initBitmap(numberOfBlocks, blockSize);
	printf("from fsInit: bitmap->fsNumBlocks:%d\n", bm->fsNumBlocks);
	mapNumBlocks = bm->mapNumBlocks;

	vcb->block_size = blockSize;
	vcb->total_blocks = numberOfBlocks;
	vcb->free_blocks = numberOfBlocks - mapNumBlocks - 2; // subtract blocks for bitmap, vcb, root
	vcb->signature = 0x1A;								  // This is an arbitrary number to check if already initialized
	vcb->fsmap_start_block = 1;
	vcb->fsmap_end_block = mapNumBlocks + 1;

	printf("From fsInit: Size of vcb:%ld, bm->fsNumBlocks before pass to initDir: %d", sizeof(vcb), bm->fsNumBlocks);

	// Initialize the root directory and LBAwrite it to disk. VCB root_start_block gets initialized
	// in the initDir function, so LBAwrite vcb must happen after.
	rootGlobal = initDir(MIN_ENTRIES, NULL, bm);
	cwdGlobal = rootGlobal; // Initialize cwdDir to rootDir
	strcpy(cwdName, "/");
	int howMany = LBAwrite(vcb, 1, 0);



	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}
