/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
 *
 * File:: directory.c
 *
 * Description:: Implements functions for Directory struct
 *
 * 
 *
 **************************************************************/

#include "directory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Initialize a directory
void initDirectory(Directory *dir) {
    for (int i = 0; i < NUM_DE; i++) {
        dir->de[i] = NULL;
    }
    return;
}

// Add a directory entry to a directory at the first null position
void addDirectoryEntry(Directory *dir, DirectoryEntry *entry) {
    for (int i = 0; i < NUM_DE; i++) {
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
    for (int i = 0; i < NUM_DE; i++) {
        if (dir->de[i] != NULL && strcmp(dir->de[i]->name, name) == 0) {
            return dir->de[i];
        }
    }
    // If we get here, the entry was not found
    return NULL;
}