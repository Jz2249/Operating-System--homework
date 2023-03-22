#ifndef _FILE_H_
#define _FILE_H_

#include "unixfilesystem.h"

/**
 * Fetches the specified file block from the specified inode and
 * stores it at buf. FileBlockIndex is the index of a payload block within
 * a file: 0 means the first payload block, 1 means the second payload block,
 * and so on, regardless of whether those payload blocks are stored directly
 * in the inode or in indirect blocks. Returns the number of valid bytes in 
 * that block (which will be the same as the sector size, except for the last 
 * block of a file).  Returns -1 if a disk error occurs or if another 
 * filesystem function this function uses returns -1.
 */
int file_getblock(const struct unixfilesystem *fs, int inumber, 
                  int fileBlockIndex, void *buf); 

#endif // _FILE_H_
