/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman
 * Student IDs:: 916867424
 * GitHub-Name:: shezgo
 * Group-Name:: Independent
 * Project:: Basic File System
*
* File:: b_io.c
*
* Description:: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int flags;		//holds the permissions value for O_RDONLY (0), O_WRONLY (1), O_RDWR (2)
	int blockTracker;	//holds the current block for tracking which can differ from start block
	int blockIndex; //holds the index inside the current block for tracking last read value
	int startBlock;	//holds the starting block of the file
	int numBytesRead;	//If this int reaches the file size, then end of file is reached.
	int eof;		// The value is 0 if EOF has not been reached, and is 1 if reached.
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY (0), O_WRONLY (1), or O_RDWR (2)
b_io_fd b_open (char * filename, int flags)
	{

	ppinfo ppi;
	int parseFlag = parsePath(filename, &ppi);

	//Check that the file exists and is not a directory.
	if(ppi.parent[ppi.lei].isDirectory)
	{
		fprintf(stderr, "b_open: path is a directory\n");
		return -1;
	}
	
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if (returnFd == -1)
	{
		fprintf(stderr, "Error: fcbArray is already full\n");
		return -1;
	}

	/*DEBUG, should the size of buf be filesize or blocksize?
	Either reading from file into user's buffer, or writing from
	user's buffer into file. Buffer should start with filesize I think.
	*/

	//Allows multiple fcb for the same file, can add mutex locks later.
	fcbArray[returnFd].buflen = ppi.parent[ppi.lei].size;
	fcbArray[returnFd].buf = malloc(ppi.parent[ppi.lei].size);//Should this be blocksize instead?
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].flags = flags;
	fcbArray[returnFd].blockTracker = ppi.parent[ppi.lei].LBAlocation;
	fcbArray[returnFd].blockIndex = 0;
	fcbArray[returnFd].startBlock = ppi.parent[ppi.lei].LBAlocation;
	fcbArray[returnFd].numBytesRead = 0;
	fcbArray[returnFd].eof = 0;

	if (fcbArray[returnFd].buf == NULL)
	{
		fprintf(stderr, "b_open: Memory allocation failed.\n");
		b_close(returnFd);
		return -1;
	}
	
	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{

	}
