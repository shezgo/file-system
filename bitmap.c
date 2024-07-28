#ifndef BITMAP_C
#define BITMAP_C

#include "bitmap.h"

// Function to LBAwrite the freespace bitmap
void mapToDisk(Bitmap *bm)
{
    LBAwrite(bm->bitmap, bm->mapNumBlocks, 1);
    return;
}
// Function to set a bit (mark block as used). Returns 1 if success, -1 if failure.
int setBit(Bitmap *bm, int blockNumber)
{
    printf("From bitmap.c->setBit: setting blockNumber: %d\n", blockNumber);
    if (blockNumber < bm->fsNumBlocks)
    {
        int byteIndex = blockNumber / 8;
        int bitIndex = blockNumber % 8;
        bm->bitmap[byteIndex] |= (1 << bitIndex);
        mapToDisk(bm);
        return 1;
    }
    else
    {
        fprintf(stderr, "Invalid blockNumber\n");
        return -1;
    }
}
// Function to clear a bit (mark block as free). Returns 1 if success, -1 if failure.
int clearBit(Bitmap *bm, int blockNumber)
{
    if (blockNumber < bm->fsNumBlocks)
    {
        int byteIndex = blockNumber / 8;
        int bitIndex = blockNumber % 8;
        bm->bitmap[byteIndex] &= ~(1 << bitIndex);
        mapToDisk(bm);
        return 1;
    }
    else
    {
        fprintf(stderr, "Invalid blockNumber\n");
        return -1;
    }
}
// Function to check if a block is 1 (used). Returns 1 if block (bit) is being used.
int isBitUsed(Bitmap *bm, int blockNumber)
{
    if (blockNumber < bm->fsNumBlocks)
    {
        int byteIndex = blockNumber / 8;
        int bitIndex = blockNumber % 8;
        return (bm->bitmap[byteIndex] & (1 << bitIndex)) != 0;
    }
    else
    {
        fprintf(stderr, "Invalid blockNumber\n");
        return -1;
    }
}

// Depending on requested number of blocks from user, send a starting block to be used.
int fsAlloc(Bitmap *bm, int req)
{
    // Check if req is a valid value
    if (req <= 0 || req > bm->fsNumBlocks)
    {
        fprintf(stderr, "Invalid request size\n");
        return -1;
    }

    int consecutiveFreeBlocks = 0;
    int startBlock = -1;

    // Iterate through each bit in the bitmap
    for (int byteIndex = 0; byteIndex < bm->bitmapSize; byteIndex++)
    {
        // If the byte is not all 1s, there's at least one free bit in it
        if (bm->bitmap[byteIndex] != 0xFF)
        {
            for (int bitIndex = 0; bitIndex < 8; bitIndex++)
            {
                int blockNumber = byteIndex * 8 + bitIndex;

                // Check if the current block is free
                if ((bm->bitmap[byteIndex] & (1 << bitIndex)) == 0)
                {
                    // If this is the first free block in the current sequence, start tracking
                    if (consecutiveFreeBlocks == 0)
                    {
                        startBlock = blockNumber;
                    }
                    consecutiveFreeBlocks++;

                    // If we found a successful sequence
                    if (consecutiveFreeBlocks == req)
                    {
                        for(int i = startBlock; i < startBlock + req; i++)
                        {
                            printf("From bitmap.c->fsAlloc: setting bit: %d\n", i);
                            setBit(bm,i); //mark the blocks as used for the space requester
                        }
                        return startBlock;
                    }
                }
                else
                {
                    // Reset if an occupied block is found before req num of blocks are found
                    consecutiveFreeBlocks = 0;
                    startBlock = -1;
                }

                // If the current block number exceeds the number of file system blocks
                if (blockNumber >= bm->fsNumBlocks)
                {
                    return -1;
                }
            }
        }
        else
        {
            // Reset if the byte is all 1s
            consecutiveFreeBlocks = 0;
            startBlock = -1;
        }
    }

    // If the entire bitmap has been traversed and no contiguous blocks were found
    return -1;
}

//Return 1 if successful, -1 if failed
int fsRelease(Bitmap* bm, int startBlock, int count)
{
    /*
    Make sure params are valid
    Set the bits in the bm to being free. Anything else?
    Return
    */
    if(bm == NULL)
    {
        fprintf(stderr, "Bitmap is null\n");
        return -1;
    }
    if(startBlock < 0 || (startBlock + count) > bm->fsNumBlocks)
    {
        fprintf(stderr, "Invalid blocks");
        return -1;
    }
    if(count < 1){
        fprintf(stderr, "Invalid count");
        return -1;
    }
    
    int clearRet;

    for(int i = startBlock; i < startBlock + count; i++)
    {
        clearRet = clearBit(bm, i);
        if(clearRet == -1){
            fprintf(stderr, "clearBit failed");
            return -1;
        }
    }
    return 1;
}

Bitmap *initBitmap(int fsNumBlocks, int blockSize)
{
    // Allocate and initialize the memory for the Bitmap struct
    Bitmap *bm = (Bitmap *)malloc(sizeof(Bitmap));
    if (bm == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for Bitmap structure\n");
        exit(EXIT_FAILURE);
    }

    // Allocate the size for bitmap - Convert the num bytes to num bits for bitmap malloc
    int blocksToBitsInBytes = (fsNumBlocks + 7) / 8;
    
    bm->fsNumBlocks = fsNumBlocks;
    //Below operation works because of int division.
    int roundedBytes = ((blocksToBitsInBytes + blockSize - 1)/ blockSize) * blockSize;
    printf("BITMAP.C>initBitmap: roundedBytes: %d\n", roundedBytes);
    bm->bitmap = (uint8_t *)malloc(roundedBytes);
    if (bm->bitmap == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for bitmap\n");
        free(bm);
        exit(EXIT_FAILURE);
    }
    bm->bitmapSize = blocksToBitsInBytes;

    // Initialize all bits in the bitmap to 0 (free)
    for (int i = 0; i < roundedBytes; i++)
    {
        bm->bitmap[i] = 0;
    }

    /*Set the bits that we know are going to be occupied. This includes the VCB in logical block 0,
    but physical block 0 contains Professor's partition table. Also set blocks needed
    to store the free space map.
    Per "steps for milestone 1 pdf" do not free the bitmap buffer. Keep it in memory and
    LBAwrite whenever you manipulate the buffer. */

    bm->mapNumBlocks = (blocksToBitsInBytes + blockSize - 1) / blockSize;
    printf("From directory_entry.h-> initBitmap: mapNumBlocks:%d\n", bm->mapNumBlocks);

    for (int i = 0; i <= bm->mapNumBlocks; i++)
    {
        setBit(bm, i);
    }

    // LBAwrite the bitmap at correct locations
    mapToDisk(bm);

    return bm;
}

#endif