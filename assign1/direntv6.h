#ifndef _DIRENTV6_H_
#define _DIRENTV6_H_

#include <stdint.h>

/**
 * The Unix Version 6 code didn't use a structure like this but this 
 * structure matches the format of a directory entry.
 */
#define MAX_COMPONENT_LENGTH 14
struct direntv6 {
    uint16_t d_inumber;
    
    /* The name corresponding to d_inumber; note: if the name is
     * MAX_COMPONENT_LENGTH long, then there is no null terminating
     * character! */
    char     d_name[MAX_COMPONENT_LENGTH];
};

#endif // _DIRENTV6_H_
