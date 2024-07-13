#include <stdint.h>
#ifndef _VOLUME_CONTROL_BLOCK_H
#define _VOLUME_CONTROL_BLOCK_H


typedef struct VolumeControlBlock
{
    uint8_t block_size;//the size of an individual block in bytes
    uint8_t total_blocks;//the total number of blocks in our volume control blocks
    uint8_t free_blocks;//current blocks that are not occupied
    uint8_t signature;//signature for file system
    uint8_t root_directory_block;//(block number) the location of the root directory)
    uint8_t fsmap_start_block;//Stores the index of first block containing the free space map
    uint8_t fsmap_end_block;//Stores the index of last block containing the free space map
} VolumeControlBlock;


#endif