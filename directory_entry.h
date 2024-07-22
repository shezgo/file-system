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

#include <time.h>
#include <stdint.h>

#define NAME 31

typedef struct DirectoryEntry
{
    long size;//size of the file
    long LBAlocation;// location where the directory entry is stored
    char name[NAME + 1];// Name of the directory entry
    time_t timeCreation;//the time the DE was created
    time_t lastAccessed;//the time the DE was last accessed
    time_t lastModified;//the time the DE was last modified
    int16_t isDirectory;//checks if it is a directory. If false and exists, it's a file.

    
} DirectoryEntry;

#endif