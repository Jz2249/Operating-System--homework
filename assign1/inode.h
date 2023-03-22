#ifndef _INODE_H
#define _INODE_H

#include "unixfilesystem.h"

/**
 * Given the i-number of a file (inumber),this function fetches from
 * disk the inode for that file and stores the inode contents at *inp.
 * Returns 0 on success, or -1 if a disk error occurs when trying to
 * read in the inode.
 */
int inode_iget(const struct unixfilesystem *fs, int inumber, 
        struct inode *inp); 

/**
 * Given the logical index of a payload block in a file, returns the block 
 * number on disk where that data is stored.  FileBlockIndex is the index of a 
 * payload block within a file: 0 means the first payload block, 1 means the 
 * second payload block, and so on, regardless of whether those payload blocks 
 * are stored directly in the inode or in indirect blocks. inp points to the 
 * inode for the file.  If a disk error occurs when trying to access a block, 
 * or if the fileBlockIndex number exceeds the maximum valid value for this 
 * inode, this function returns -1.
 */
int inode_indexlookup(const struct unixfilesystem *fs, struct inode *inp,
        int fileBlockIndex);

/**
 * Given an inode, this function computes the size of its file from the
 * size0 and size1 fields in the inode.
 */
int inode_getsize(struct inode *inp);

#endif // _INODE_
