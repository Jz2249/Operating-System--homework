#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

#include "unixfilesystem.h"
#include "direntv6.h"

/**
 * Looks up the specified name (name) in the directory whose inumber
 * is dirinumber. If found, stores the directory entry contents at *dirEnt. 
 * Returns 0 on success, or -1 if not found, if a disk error occurs or if 
 * another filesystem function this function uses returns -1.
 */
int directory_findname(const struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt);
   
#endif // _DIRECTORY_H_
