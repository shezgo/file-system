/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
 *
 * File:: directory.h
 *
 * Description:: Header file for Directory struct.
 *
 * 
 *
 **************************************************************/

#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#include "directory_entry.h"
#include <stdint.h>

#define NUM_DE 50 // initial number of directory entries held in a directory
typedef struct Directory
{
    //char * name[NAME + 1]; is this needed?
    struct DirectoryEntry *de[NUM_DE];

} Directory;

void initDirectory(Directory *dir); //Set all DE to null
void addDirectoryEntry(Directory *dir, DirectoryEntry *entry); 
DirectoryEntry *findDirectoryEntry(Directory *dir, const char *name);

#endif