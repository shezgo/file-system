/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
 *
 * File:: directory_entry.h
 *
 * Description:: Holds DirectoryEntry struct
 *
 *
 *
 **************************************************************/

#ifndef _DIRECTORY_ENTRY_H
#define _DIRECTORY_ENTRY_H

#include "volume_control_block.h"
#include "bitmap.c"
#include <time.h>
#include <stdint.h>

#define NAME 31

typedef struct DE
{
    long size;           // size of the file
    long LBAlocation;    // location where the directory entry is stored
    long logicalAddress; // index within the block
    char name[NAME + 1]; // Name of the directory entry
    time_t timeCreation; // the time the DE was created
    time_t lastAccessed; // the time the DE was last accessed
    time_t lastModified; // the time the DE was last modified
    int16_t isDirectory; // checks if it is a directory. 1 if directory, 0 if not.

} DE;

// Since this is a pointer, we'll want this loaded into RAM. For root dir, parent will point to null
// so just pass in null.
DE *initDir(int minEntries, DE *parent, VolumeControlBlock *vcb, Bitmap *bm)
{
    int BLOCKSIZE = vcb->block_size;
    int bytesNeeded = minEntries * sizeof(DE);
    int blocksNeeded = (bytesNeeded + BLOCKSIZE - 1) / BLOCKSIZE;
    int bytesToAlloc = blocksNeeded * BLOCKSIZE;

    // Allocate memory for the directory
    DE *newDir = malloc(bytesToAlloc);

    // Calculate number of entries that can fit inside the directory block(s)
    int actualEntries = bytesToAlloc / sizeof(DE);

    // Initialize the entries in the directory - start with 2 because . and .. are at 0 and 1
    for (int i = 2; i < actualEntries; i++)
    {
        newDir[i].size = -1;
        newDir[i].LBAlocation = -1;
        newDir[i].logicalAddress = -1;
        newDir[i].name[0] = '\0';
        newDir[i].timeCreation = (time_t)(-1);
        newDir[i].lastAccessed = (time_t)(-1);
        newDir[i].lastModified = (time_t)(-1);
        newDir[i].isDirectory = -1;
    }
    // Initialize . entry in the directory
    int newLoc = fsAlloc(bm, blocksNeeded);
    printf("From directory_entry.h->initDir: newLoc:%d blocksNeeded:%d\n", newLoc, blocksNeeded);
    time_t tc = time(NULL);
    newDir[0].LBAlocation = newLoc;
    newDir[0].logicalAddress = 0; // Every new directory starts at beginning block boundary
    newDir[0].size = actualEntries * sizeof(DE);
    strcpy(newDir[0].name, ".");
    newDir[0].isDirectory = 1;
    newDir[0].timeCreation = tc;
    newDir[0].lastAccessed = tc;
    newDir[0].lastModified = tc;

    // Initialize the .. (parent) entry in the directory. This inits root correctly as well.
    DE *dotdot = parent;

    // Root case
    if (dotdot == NULL)
    {
        dotdot = newDir;
        vcb->root_directory_block = newDir->LBAlocation;
    }

    memcpy(&newDir[1], dotdot, sizeof(DE));
    strcpy(newDir[1].name, "..");
    newDir[1].logicalAddress = 1 * sizeof(DE);

    LBAwrite(newDir, blocksNeeded, newLoc);
    
    return newDir;
}

#endif