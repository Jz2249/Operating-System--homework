/**
 * File: catn.c
 * --------------
 * This program prints the first nbytes bytes of a specified file,
 * using the open/read/close system calls.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


/* This function prints the first nbytes bytes of the given file
 * (assumed to be text).  If nbytes is larger than the file size,
 * it just prints the entire file.
 */
void printFirstNBytes(int sourceFD, int nbytes) {
  // Make space for the chars + null terminator
  char buffer[nbytes + 1];
  size_t bytesRead = 0;
  while (bytesRead < nbytes) {
    // Read the next chunk of bytes from the source
    ssize_t bytesReadThisTime = read(sourceFD, buffer + bytesRead, nbytes - bytesRead);
    if (bytesReadThisTime == 0) break;
    bytesRead += bytesReadThisTime;
  }

  // We must add a null terminator so it prints without memory errors
  buffer[bytesRead] = '\0';
  printf("%s\n", buffer);
}


int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: ./catn <filename> <nbytes>\n");
    return 1;
  }

  int sourceFD = open(argv[1], O_RDONLY);
  int nbytes = atoi(argv[2]);
  printFirstNBytes(sourceFD, nbytes);
  return 0;
}
