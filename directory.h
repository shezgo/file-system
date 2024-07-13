#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#include "directory_entry.h"
#include <stdint.h>

#define NUM_DE 10 //initial number of directory entries held in a directory

typedef struct Directory
{
        struct DirectoryEntry *de[NUM_DE];
    
} Directory;

#endif