#ifndef _PATHNAME_H_
#define _PATHNAME_H_

#include "unixfilesystem.h"

/**
 * Returns the inumber associated with the specified pathname.  This
 * function assumes the path is absolute (starting with a /).  Returns -1 if 
 * the path is not valid, if a disk error occurs or if another filesystem 
 * function this function uses returns -1.
 */
int pathname_lookup(const struct unixfilesystem *fs, const char *pathname);

#endif // _PATHNAME_H_
