
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>

int pathname_lookup(const struct unixfilesystem *fs, const char *pathname) {
    // Remove the placeholder code below and add your implementation.
    if (strlen(pathname) == 1) { // if there is only "/"
        return ROOT_INUMBER;
    }

    char path_copy[strlen(pathname) + 1];
    strcpy(path_copy, pathname);
    char *cpy = path_copy;
    int inum = ROOT_INUMBER;
    char* temp = strsep(&cpy, "/"); 
    struct direntv6 dirEnt;

    while ((temp = strsep(&cpy, "/")) != NULL) {
        int sign = directory_findname(fs, temp, inum, &dirEnt);
        if (sign == -1) {
            fprintf(stderr, "pathname_lookup(pathname=\"%s\") presented a "
                "pathanme with a bogus component of \"%s\"... "
                "returning -1.\n", pathname, temp);
            return -1;
        }
        inum = dirEnt.d_inumber;
    }
    return inum;
}
