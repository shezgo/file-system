#ifndef _DIRECTORY_ENTRY_H
#define _DIRECTORY_ENTRY_H

#include <time.h>
#include <stdint.h>

#define NAME 50
#define NUM_DE 10 //initial number of directory entries hel

typedef struct DirectoryEntry
{
    char name[NAME + 1];// Name of the directory entry
    uint32_t LBAlocation;// location where the directory entry is stored
    time_t timeCreation;//the time the DE was created
    time_t lastAccessed;//the time the DE was last accessed
    time_t lastModified;//the time the DE was last modified
    uint32_t size;//size of the DE
    int16_t isDirectory;//checks if it is a directory. If false and exists, it's a file.
    //add array of directory entries here
    int numDE = NUM_DE;
    struct DirectoryEntry *de[NUM_DE];
    
} DirectoryEntry;

#endif