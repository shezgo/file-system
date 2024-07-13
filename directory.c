#include "directory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize a directory
void initDirectory(Directory *dir) {
    for (int i = 0; i < NUM_DE; ++i) {
        dir->de[i] = NULL;
    }
    return;
}

// Add a directory entry to a directory
void addDirectoryEntry(Directory *dir, DirectoryEntry *entry) {
    for (int i = 0; i < NUM_DE; ++i) {
        if (dir->de[i] == NULL) {
            dir->de[i] = entry;
            return;
        }
    }
    // If we get here, the directory is full
    fprintf(stderr, "Error: Directory is full, cannot add entry.\n");
}

// Find a directory entry by name
DirectoryEntry* findDirectoryEntry(Directory *dir, const char *name) {
    for (int i = 0; i < NUM_DE; ++i) {
        if (dir->de[i] != NULL && strcmp(dir->de[i]->name, name) == 0) {
            return dir->de[i];
        }
    }
    // If we get here, the entry was not found
    return NULL;
}