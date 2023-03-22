#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>


int directory_findname(const struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt) {
    // Remove the following placeholder implementation and replace
    // with your own implementation.

    struct inode node;
    int sign = inode_iget(fs, dirinumber, &node);
    if (sign == -1) {
            return -1;
    }
    int sz = inode_getsize(&node);
    int max_index = (sz - 1) / DISKIMG_SECTOR_SIZE;

    struct direntv6 tempEnt[DISKIMG_SECTOR_SIZE / sizeof(struct direntv6)];
    for (int i = 0; i < max_index + 1; i++) {
        int blksz = file_getblock(fs, dirinumber, i, tempEnt);
        if (blksz == -1) {
            return -1;
        }
        int num = blksz / sizeof(struct direntv6);
        for (int j = 0; j < num; j++) {
            if (!strncmp(tempEnt[j].d_name, name, MAX_COMPONENT_LENGTH)) {
                *dirEnt = tempEnt[j];
                return 0;
            }
        }
    }
    return -1;
}
