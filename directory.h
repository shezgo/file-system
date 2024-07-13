#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#include "directory_entry.h"
#include <stdint.h>

#define NUM_DE 50 // initial number of directory entries held in a directory
#define NAME 50
typedef struct Directory
{
    //char * name[NAME]; is this needed?
    struct DirectoryEntry *de[NUM_DE];

} Directory;

void initDirectory(Directory *dir);
void addDirectoryEntry(Directory *dir, DirectoryEntry *entry);
DirectoryEntry *findDirectoryEntry(Directory *dir, const char *name);

#endif