#ifndef _DIRECTORY_ENTRY_C
#define _DIRECTORY_ENTRY_C

#include <time.h>
#include <stdint.h>

#define NAME 100

typedef struct DirectoryEntry
{
    char name[NAME + 1];// Name of the directory entry
    uint32_t LBAlocation;// location where the directory entry is stored
    time_t timeCreation;//the time the DE was created
    uint32_t size;//size of the DE
    int16_t isDirectory;//checks if it is a directory. If false and exists, it's a file.
} DirectoryEntry;

#endif