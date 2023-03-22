#include <stdio.h>

#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(const struct unixfilesystem *fs, int inumber,
        int fileBlockIndex, void *buf) {
        // Remove the following placeholder code and add your implementation
        struct inode node;
        int sign = inode_iget(fs, inumber, &node);
        if (sign == -1) {
                return -1;
        }
        int blkNum = inode_indexlookup(fs, &node, fileBlockIndex);
        if (blkNum == -1) {
                return -1;
        }
        int read_block = diskimg_readsector(fs->dfd, blkNum, buf);
        if (read_block == -1) {
                return -1;
        }
        int sz = inode_getsize(&node);
        int max_blk = sz / DISKIMG_SECTOR_SIZE;
        if (fileBlockIndex == max_blk) {
                return sz % DISKIMG_SECTOR_SIZE;
        }
        return DISKIMG_SECTOR_SIZE;
}
