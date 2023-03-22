/* This file is a demo of file descriptors being
 * copied by a call to fork and what the state
 * of the file descriptor table and open file
 * table look like.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static const int FILE_CHUNK_SIZE = 16;

/* This function reads the next `amount` bytes of the file
 * specified by the given file descriptor, and prints out
 * what was read in, prefixed with the `who` string.
 */
void readPartOfFile(int fd, int amount, const char *who) {
    char buf[amount + 1];
    size_t bytesRead = read(fd, buf, sizeof(buf) - 1);
    buf[bytesRead] = '\0';
    printf("%s: [%s]\n", who, buf);
}

int main() {
    int codeFile = open("open-file-table.cc", O_RDONLY);

    // Read the first part of the file
    readPartOfFile(codeFile, FILE_CHUNK_SIZE, "parent");

    pid_t pidOrZero = fork();
    if (pidOrZero == 0) {
        // The child will read another chunk of the file
        readPartOfFile(codeFile, FILE_CHUNK_SIZE, "child");
        close(codeFile);
        return 0;
    }

    // The parent waits for the child, and then reads again
    waitpid(pidOrZero, NULL, 0);
    readPartOfFile(codeFile, FILE_CHUNK_SIZE, "parent");
    close(codeFile);
    return 0;
}