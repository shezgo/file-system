/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
 *
 * File:: directory_entry.c
 *
 * Description::
 *	This is used for directory_entry operations
 *
 *
 *
 **************************************************************/
#include "directory_entry.h"
#include "fsInit.h"

DE *initDir(int maxEntries, DE *parent, Bitmap *bm)
{
    int BLOCKSIZE = vcb->block_size;
    int bytesNeeded = maxEntries * sizeof(DE);
    int blocksNeeded = (bytesNeeded + BLOCKSIZE - 1) / BLOCKSIZE;
    int bytesToAlloc = blocksNeeded * BLOCKSIZE;

    // Allocate memory for the directory
    DE *newDir = (DE *)malloc(bytesToAlloc);
    if (newDir == NULL)
    {
        fprintf(stderr, "New directory memory allocation failed\n");
        return NULL;
    }

    // Initialize all bytes to 0 using a for loop
    for (uint32_t i = 0; i < bytesToAlloc; i++)
    {
        ((char *)newDir)[i] = 0;
    }

    // Calculate number of entries that can fit inside the directory block(s)
    int actualEntries = bytesToAlloc / sizeof(DE);
    printf("From directory_entry.c->initDir:BEFORE fsAlloc:bm->fsNumBlocks:%d, blocksNeeded:%d\n", bm->fsNumBlocks, blocksNeeded);
    int newLoc = fsAlloc(bm, blocksNeeded);
    printf("From directory_entry.c->initDir: newLoc:%d bm->fsNumBlocks:%d, blocksNeeded:%d\n", newLoc, bm->fsNumBlocks, blocksNeeded);
    int entriesPerBlock = actualEntries / blocksNeeded; // Old code

    // Assign LBA locations to each directory entry
    for (int i = 0; i < actualEntries; i++)
    {
        int blockOffset = i / (BLOCKSIZE / sizeof(DE)); // Determine the block this entry belongs to
        newDir[i].LBAlocation = newLoc + blockOffset;
        newDir[i].LBAindex = (i % entriesPerBlock) * sizeof(DE);
        newDir[i].size = -1;
        newDir[i].name[0] = '\0';
        newDir[i].timeCreation = (time_t)(-1);
        newDir[i].lastAccessed = (time_t)(-1);
        newDir[i].lastModified = (time_t)(-1);
        newDir[i].isDirectory = -1;
        newDir[i].dirNumBlocks = -1;
    }

    /*  Old Code
        // DEBUG confirm that this works
        // Init LBAlocations depending on how many entries per block there are.

        for (int i = 0, k = newLoc; i < actualEntries; i += entriesPerBlock, k++)
        {
            for (int j = 0; j < entriesPerBlock; j++)
            {
                if ((i + j) < actualEntries)
                {
                    newDir[i + j].LBAlocation = k;
                }
                else
                {
                    break;
                }
            }
        }
        */

    /* More old code
     // Initialize the entries in the directory - start with 2 because . and .. are at 0 and 1
     for (int i = 2; i < actualEntries; i++)
     {
         newDir[i].size = -1;
         newDir[i].name[0] = '\0';
         newDir[i].timeCreation = (time_t)(-1);
         newDir[i].lastAccessed = (time_t)(-1);
         newDir[i].lastModified = (time_t)(-1);
         newDir[i].isDirectory = -1;
         newDir[i].dirNumBlocks = -1;
     }
     /*
         for the first entriesPerBlock, assign newLoc + 0to LBAlocation
         for entriesPerBlock to 2* entriesPerBlock, assign newLoc + 1 to LBA location
         for 2*entriesPerBlock to 3*entriesPerBlock, assign newLoc + 2 to LBA location
     */

    // Initialize . entry in the directory
    printf("From directory_entry.h->initDir: newLoc:%d blocksNeeded:%d\n", newLoc, blocksNeeded);
    time_t tc = time(NULL);
    printf("directory_entry tc test:%ld\n", tc);
    newDir[0].size = actualEntries * sizeof(DE);
    strcpy(newDir[0].name, ".");
    newDir[0].isDirectory = 1;
    newDir[0].timeCreation = tc;
    newDir[0].lastAccessed = tc;
    newDir[0].lastModified = tc;
    newDir[0].dirNumBlocks = blocksNeeded;

    // Initialize the .. (parent) entry in the directory. This inits root correctly as well.
    DE *dotdot = parent;

    // Root case
    if (dotdot == NULL)
    {
        dotdot = newDir;
        vcb->root_directory_block = newDir->LBAlocation;
        vcb->root_num_blocks = blocksNeeded;
    }

    memcpy(&newDir[1], dotdot, sizeof(DE));
    strcpy(newDir[1].name, "..");

    LBAwrite(newDir, blocksNeeded, newLoc);

    return newDir;
}

// Loads a directory into memory for manipulation. Currently loads all bytes in the shared LBA
// in addition to the desired directory.
// DEBUG: ensure root case is being handled
DE *loadDirDE(DE *dir)
{
    if (dir == NULL)
    {
        fprintf(stderr, "Cannot load NULL dir\n");
        return NULL;
    }
    if (dir->isDirectory == 0)
    {
        fprintf(stderr, "loadDir: DE is not a directory.\n");
        return NULL;
    }
    // DEBUG no name for root
    if (strcmp(dir->name, rootGlobal->name) == 0)
    {
        return rootGlobal;
    }

    // New code starts here
    // DEBUG storing dirNumBlocks is redundant
    void *buffer = malloc(dir->dirNumBlocks * vcb->block_size);

    if (buffer == NULL)
    {
        perror("Failed to allocate for buffer in loadDir\n");
        exit(EXIT_FAILURE);
    }
    // this is correct
    int readReturn = LBAread(buffer, dir->dirNumBlocks, dir->LBAlocation);

    if (readReturn != dir->dirNumBlocks)
    {
        perror("Failed to loadDirDE\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    return (DE *)buffer;
}

DE *loadDirLBA(int numBlocks, int startBlock)
{

    // New code starts here
    void *buffer = malloc(numBlocks * vcb->block_size);

    if (buffer == NULL)
    {
        perror("Failed to allocate for buffer in loadDir\n");
        exit(EXIT_FAILURE);
    }

    int readReturn = LBAread(buffer, numBlocks, startBlock);

    if (readReturn != numBlocks)
    {
        perror("Failed to loadDirLBA\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    return (DE *)buffer;
}

// Update a directory on disk. Returns 1 on success, -1 if failed.
// This should ONLY update a DE on disk from an entire loaded directory in memory
// PICKUP/DEBUG: Can I ensure this^ with reading into a buffer and freeing within this method?
//

int updateDELBA(DE *dir)
{
    /*
        To update a directory, rewrite it to its LBAlocation.
        Update lastAccessed and lastModified in directory[0]
        AND in its parent directory at correct index.

    */

    void *buffer = malloc(vcb->block_size * DIRECTORY_NUM_BLOCKS);
    if (buffer == NULL)
    {
        perror("Failed to allocate for buffer in updateDELBA\n");
        exit(EXIT_FAILURE);
    }

    int readReturn = LBAread(buffer, DIRECTORY_NUM_BLOCKS, dir->LBAlocation);
    if (readReturn < 0)
    {
        perror("Failed to updateDELBA \n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    DE *completeDir = (DE *)buffer;
    if (completeDir == NULL)
    {
        fprintf(stderr, "Directory is null\n");
        return -1;
    }
    if (completeDir[0].isDirectory == 0)
    {
        fprintf(stderr, "updateDELBA: DE is not a directory.\n");
        return 1;
    }

    time_t tc = time(NULL);
    completeDir[0].lastAccessed = tc;
    completeDir[0].lastModified = tc;

    int ret = LBAwrite(completeDir, completeDir[0].dirNumBlocks, completeDir[0].LBAlocation);
    if (ret != 1)
    {
        perror("Failed to updateDELBA \n");
        free(completeDir);
        exit(EXIT_FAILURE);
    }

    // need to allocate memory for the parent directory first though
    DE *parent = (DE *)malloc(vcb->block_size * DIRECTORY_NUM_BLOCKS);
    if (parent == NULL)
    {
        perror("Failed to allocate for parent buffer in updateDELBA\n");
        exit(EXIT_FAILURE);
    }

    parent = loadDirDE(&(completeDir[1]));
    int parentIndex = findNameInDir(parent, completeDir[0].name);
    // Now, edit the metadata inside the parent
    // and rewrite the parent directory to disk

    parent[parentIndex].lastAccessed = tc;
    parent[parentIndex].lastModified = tc;
    ret = LBAwrite(parent, parent[0].dirNumBlocks, parent[0].LBAlocation);
    if (ret != 1)
    {
        perror("Failed to updateDELBA \n");
        free(parent);
        free(completeDir);
        exit(EXIT_FAILURE);
    }

    free(parent);
    free(completeDir);
    return ret;
}
