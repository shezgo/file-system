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
#include "bitmap.h"
#include <time.h>
#include <stdint.h>
#include <string.h>

#define NAME 31

//To find the logical address of a DE within its parent directory, just use
//index in its parent directory * sizeof(DE).
typedef struct DE
{
    long size;           // size of the file in bytes, or actualEntries * sizeof(DE) if directory
    long LBAlocation;    // location where the directory entry is stored
    char name[NAME + 1]; // Name of the directory entry
    time_t timeCreation; // the time the DE was created
    time_t lastAccessed; // the time the DE was last accessed
    time_t lastModified; // the time the DE was last modified

    int16_t isDirectory; // checks if it is a directory. 1 if directory, 0 if not.
    int32_t dirNumBlocks; // Number of blocks if it's a directory. If not dir or null, -1.


} DE;

// Since this is a pointer, we'll want this loaded into RAM. For root dir, parent will point to null
// so just pass in null.
DE *initDir(int minEntries, DE *parent, VolumeControlBlock *vcb, Bitmap *bm);

#endif