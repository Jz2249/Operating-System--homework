#include <stdio.h>

#include "inode.h"
#include "diskimg.h"
#include <stdbool.h>

#define INODE_PER_BLOCK 16
#define NUM_INDIRECT_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))
#define MAX_ADDR_INDEX 7

int inode_iget(const struct unixfilesystem *fs, int inumber,
        struct inode *inp) {
    // Remove the placeholder code below and add your implementation.
    int sectorNum = (inumber - 1) / INODE_PER_BLOCK + INODE_START_SECTOR;
    struct inode inodes[INODE_PER_BLOCK];
    int indexNum = (inumber - 1) % INODE_PER_BLOCK;
    int sign = diskimg_readsector(fs->dfd, sectorNum, inodes);
    if (sign == -1) {
        return -1;
    }
    *inp = inodes[indexNum];
    return 0;
}

int inode_indexlookup(const struct unixfilesystem *fs, struct inode *inp,
        int fileBlockIndex) {
    // Remove the placeholder code below and add your implementation.
    int sz = inode_getsize(inp);
    if (fileBlockIndex > ((sz - 1) / DISKIMG_SECTOR_SIZE)) {
        return -1;
    }
    if ((inp->i_mode & ILARG) == 0) { // direct block
        return inp->i_addr[fileBlockIndex];
    }
    uint16_t inodes1[NUM_INDIRECT_PER_BLOCK];
    int blkIndex = fileBlockIndex / NUM_INDIRECT_PER_BLOCK;
    int index = fileBlockIndex % NUM_INDIRECT_PER_BLOCK;
    if (blkIndex < MAX_ADDR_INDEX) { // singly indirect
        int sectorNum = inp->i_addr[blkIndex];
        if (diskimg_readsector(fs->dfd, sectorNum, inodes1) == -1) {
            return -1;
        }
        return inodes1[index];
    }
    else { // double indirect
        int tableIdx = blkIndex - MAX_ADDR_INDEX;
        int sectorNum = inp->i_addr[MAX_ADDR_INDEX];
        if (diskimg_readsector(fs->dfd, sectorNum, inodes1) == -1) {
            return -1;
        }
        sectorNum = inodes1[tableIdx];
        uint16_t inodes2[NUM_INDIRECT_PER_BLOCK];
        if (diskimg_readsector(fs->dfd, sectorNum, inodes2) == -1) {
            return -1;
        }
        return inodes2[index];
    }
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1);
}
