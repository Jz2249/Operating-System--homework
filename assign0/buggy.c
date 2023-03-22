/* File: buggy.c
 * --------------
 * For each command line argument, this program prints out the number of blocks
 * of BLOCK_SIZE characters in that argument, along with what the last block
 * is.  If an argument isn't a multiple of BLOCK_SIZE characters long, it
 * prints a message to that effect instead.  For instance, if you run 
 *
 *  ./buggy hello cs111!
 *
 * It will print that hello is not a multiple of 3 characters, and will print
 * that cs111! is 2 blocks of 3 characters long, the last of which is '11!'.
 * 
 * However, there is a bug in the implementation below...
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 3


/* FUNCTION: lastN
 * ------------------
 * This function takes in a string 'input' and gets just its last n
 * characters (plus the null terminator), and copies them into 'dest',
 * which is assumed to be a memory space large enough to store them.
 * Then it returns the number of blocks of n characters in 'input'.  If
 * if the length of 'input' isn't a multiple of n, this function does
 * nothing and returns -1.
 */
int lastN(const char *input, char *last, int n) {
    if (strlen(input) % n != 0) {
        return -1;
    }
    
    strcpy(last, input + strlen(input) - n); 
    return strlen(input) / n;
}

int main(int argc, char *argv[]) {
    // Test on each argument specified by the user
    for (int i = 1; i < argc; i++) {
        char buf[strlen(argv[i]) + 1];
        int numBlocksOfN = lastN(argv[i], buf, BLOCK_SIZE);
        if (numBlocksOfN == -1) {
            printf("'%s' isn't a multiple of %d characters long\n", argv[i], BLOCK_SIZE);
        } else {
            printf("'%s' contains %d blocks of %d characters; the last one is '%s'\n", 
                argv[i], numBlocksOfN, BLOCK_SIZE, buf);
        }
    }
    
    return 0;
}
