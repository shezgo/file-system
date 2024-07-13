/**************************************************************
 * Class::  CSC-415-01 Summer 2024
 * Name:: Shez Rahman, Austin Kuykendall, Robel Ayelew, Awet Fikadu
 * Student IDs:: 916867424, 920222066, 922419937, 922130310
 * GitHub-Name:: shezgo
 * Group-Name:: Spork
 * Project:: Basic File System
*
* File:: b_io.h
*
* Description:: Interface of basic I/O Operations
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

