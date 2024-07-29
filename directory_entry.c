#include "directory_entry.h"

DE *initDir(int minEntries, DE *parent, VolumeControlBlock *vcb, Bitmap *bm)
{
    int BLOCKSIZE = vcb->block_size;
    int bytesNeeded = minEntries * sizeof(DE);
    int blocksNeeded = (bytesNeeded + BLOCKSIZE - 1) / BLOCKSIZE;
    int bytesToAlloc = blocksNeeded * BLOCKSIZE;

    // Allocate memory for the directory
    DE *newDir = malloc(bytesToAlloc);

    // Calculate number of entries that can fit inside the directory block(s)
    int actualEntries = bytesToAlloc / sizeof(DE);
    int newLoc = fsAlloc(bm, blocksNeeded);
    int entriesPerBlock = actualEntries / blocksNeeded;

    //Init LBAlocations depending on how many entries per block there are.
    for (int i = 0; i < entriesPerBlock; i++)
    {
        for (int j = 0; j < blocksNeeded; j++)
        {
            newDir[j * entriesPerBlock].LBAlocation = newLoc + j;
        }
    }
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
    }

    memcpy(&newDir[1], dotdot, sizeof(DE));
    strcpy(newDir[1].name, "..");

    LBAwrite(newDir, blocksNeeded, newLoc);

    return newDir;
}
