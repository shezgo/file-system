/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman
 * Student IDs:: 916867424
 * GitHub-Name:: shezgo
 * Group-Name:: Independent
 * Project:: Basic File System
 *
 * File:: volume_control_block.c
 *
 * Description:: Holds VolumeControlBlock struct and functions
 *
 * 
 *
 **************************************************************/
#include "volume_control_block.h"

VolumeControlBlock *loadVCBtoMem(uint64_t blockSize)
{
    void *buffer = malloc(blockSize);
    if (buffer == NULL)
    {
        perror("Failed to load VCB to memory\n");
        exit(EXIT_FAILURE);
    }

    int readReturn = LBAread(buffer, 1, 0); // Read VCB from block 0
    if (readReturn != 1)
    {
        perror("Failed to read VCB from disk\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    return (VolumeControlBlock *)buffer;
}

int writeVCBtoDisk(VolumeControlBlock *vcb)
{
    if (vcb == NULL)
    {
        perror("VCB is null - cannot write to disk");
        exit(EXIT_FAILURE);
    }

    int writeReturn = LBAwrite(vcb, 1, 0);
    if (writeReturn != 1)
    {
        perror("Failed to write VCB to disk\n");
        exit(EXIT_FAILURE);
    }
    return writeReturn;
}