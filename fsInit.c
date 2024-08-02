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
VolumeControlBlock *vcb = NULL; // Global definition, always kept in memory
DE *rootGlobal = NULL;			// Global definition, always kept in memory
DE *cwdGlobal = NULL;			// Global definition, always kept in memory
char *cwdName = NULL;			// Global char* used to track cwd path string
Bitmap *bm = NULL;				// Global declaration of the freespace bitmap

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	// This error check guarantees that the vcb can fit in block 0.
	if (blockSize < sizeof(VolumeControlBlock))
	{
		fprintf(stderr, "Error: Block size (%ld) is less than sizeof(VolumeControlBlock) (%ld)\n",
				blockSize, sizeof(VolumeControlBlock));
		exit(EXIT_FAILURE);
	}

	vcb = loadVCBtoMem(blockSize);

	// If the file system has already been mounted,  read in all variables into
	// memory so they are initialized.
	if (vcb->signature == 0x1A)
	{
		bm = loadBMtoMem(blockSize);
		rootGlobal = initDir(MIN_ENTRIES, NULL, bm);
		//Always start cwd from root when starting up file system.
		cwdGlobal = rootGlobal; 

		// Initialize a global current working directory name string
		cwdName = (char *)malloc(CWD_SIZE);
		if (cwdName == NULL)
		{
			fprintf(stderr, "cwdName memory allocation failed\n");
			return -1;
		}
		for (uint32_t i = 0; i < CWD_SIZE; i++)
		{
			cwdName[i] = '\0';
		}
		strcpy(cwdName, "/");
		// end init cwdName


		
		return 1;
	}

	// If the volume hasn't been initialized
	//  Clear all memory for vcb before initializing and writing
	for (uint32_t i = 0; i < blockSize; i++)
	{
		((char *)vcb)[i] = 0;
	}

	// Initialize Global pointer to keep track of current working directory
	cwdName = (char *)malloc(CWD_SIZE);
	if (cwdName == NULL)
	{
		fprintf(stderr, "cwdName memory allocation failed\n");
		return -1;
	}
	for (uint32_t i = 0; i < CWD_SIZE; i++)
	{
		cwdName[i] = '\0';
	}

	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks,
		   blockSize);

	bm = initBitmap(numberOfBlocks, blockSize);

	vcb->block_size = blockSize;
	vcb->total_blocks = numberOfBlocks;
	vcb->free_blocks = vcb->total_blocks - vcb->fsmap_num_blocks - 2; // subtract blocks for bitmap, vcb, root
	vcb->signature = 0x1A;								  // This is an arbitrary number to check if already initialized
	vcb->fsmap_start_block = 1;
	vcb->fsmap_end_block = vcb->fsmap_num_blocks + 1;
	vcb->fsmap_num_blocks = (vcb->fsmap_end_block - vcb->fsmap_start_block + 1);

	// Initialize and write root directory to disk. VCB root_start_block gets initialized
	// in the initDir function, so writing vcb to disk must happen after.
	rootGlobal = initDir(MIN_ENTRIES, NULL, bm);
	//Current working directory always starts at root when starting file system.
	cwdGlobal = rootGlobal; 
	strcpy(cwdName, "/");
	int vcbReturn = LBAwrite(vcb, 1, 0);

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}
