#include "volume_control_block.h"
#include <stdlib.h>

VolumeControlBlock* loadVCBtoMem(uint64_t blockSize) {
    void *buffer = malloc(blockSize);
    if (buffer == NULL) {
        perror("Failed to load VCB to memory\n");
        exit(EXIT_FAILURE);
    }

    int readReturn = LBAread(buffer, 1, 0); // Read VCB from block 0

    return (VolumeControlBlock *)buffer;
}

int writeVCBtoDisk(VolumeControlBlock *vcb)
{
    if(vcb == NULL){
        perror("VCB is null - cannot write to disk");
        exit(EXIT_FAILURE);
    }

    int writeReturn = LBAwrite(vcb, 1, 0);
    return writeReturn;
}