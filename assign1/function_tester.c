#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "diskimg.h"
#include "unixfilesystem.h"
#include "inode.h"
#include "file.h"
#include "directory.h"
#include "pathname.h"

/* This function tests the inode_iget function; it error checks for if
 * the specified inumber is invalid for this filesystem, and otherwise
 * prints the return value of inode_iget along with the fields in the inode
 * that was read in and what they say about the inode (e.g. directory,
 * what mapping scheme, free or allocated).
 */
static void test_inode_iget(const struct unixfilesystem *fs, int inumber) {
  // Bounds check
  int max_inode_number = fs->superblock.s_isize * 16;
  if (inumber < 1 || inumber > max_inode_number) {
    printf("ERROR: invalid inumber for this disk; ");
    printf("must be between 1 and %d, inclusive\n", max_inode_number);
    return;
  }

  // Read in the inode
  struct inode in;
  int result = inode_iget(fs, inumber, &in);
  if (result == -1) {
    printf("inode_iget(%d) returned -1\n", inumber);
    return;
  }

  // Print out the inode fields
  printf("struct inode {\n");
  printf("\ti_mode = %d,\n", in.i_mode);
  printf("\ti_nlink = %d,\n", in.i_nlink);
  printf("\ti_uid = %d,\n", in.i_uid);
  printf("\ti_gid = %d,\n", in.i_gid);
  printf("\tsize = %d,\n", inode_getsize(&in));
  for (size_t i = 0; i < sizeof(in.i_addr) / sizeof(in.i_addr[0]); i++) {
    printf("\ti_addr[%lu] = %d,\n", i, in.i_addr[i]);
  }
  printf("\ti_atime = %d %d,\n", in.i_atime[0], in.i_atime[1]);
  printf("\ti_mtime = %d %d\n", in.i_mtime[0], in.i_mtime[1]);
  printf("}\n\n");

  // Print inode information from its i_mode
  if ((in.i_mode & IALLOC) != 0) {
    printf("This inode is in use.\n");

    if ((in.i_mode & IFMT) == IFDIR) {
      printf("This inode represents a directory.\n");
    } else {
      printf("This inode does not represent a directory.\n");
    }

    if ((in.i_mode & ILARG) != 0) {
      printf("This inode uses the large mapping scheme.\n");
    } else {
      printf("This inode uses the small mapping scheme.\n");
    }
  } else {
    printf("This inode is free.\n");
  }
}

/* This function tests the inode_indexlookup function; it error checks for if
 * the specified inumber is invalid for this filesystem, if this inode
 * is free (and therefore we cannot read its block numbers), or if inode_iget
 * returned -1, and otherwise prints the return value of inode_indexlookup.
 */
static void test_inode_indexlookup(const struct unixfilesystem *fs, 
    int inumber, int fileBlockIndex) {
  
  // Bounds check
  int max_inode_number = fs->superblock.s_isize * 16;
  if (inumber < 1 || inumber > max_inode_number) {
    printf("ERROR: invalid inumber for this disk; ");
    printf("must be between 1 and %d, inclusive\n", max_inode_number);
    return;
  }

  // Read in the inode
  struct inode in;
  int result = inode_iget(fs, inumber, &in);
  if (result == -1) {
    printf("inode_iget(%d) returned -1\n", inumber);
    return;
  }

  if ((in.i_mode & IALLOC) == 0) {
    printf("ERROR: this inode is marked free, can't access block numbers.\n");
    return;
  }
  
  result = inode_indexlookup(fs, &in, fileBlockIndex);
  printf("inode_indexlookup(inumber = %d, fileBlockIndex = %d) returned %d\n", 
    inumber, fileBlockIndex, result);
}

/* This function tests the file_getblock function; it error checks for if
 * the specified inumber is invalid for this filesystem, if this inode
 * is free (and therefore we cannot read its blocks), or if inode_iget
 * returned -1, and otherwise prints the return value of inode_indexlookup
 * and the contents of the block in hexadecimal.
 */
static void test_file_getblock(const struct unixfilesystem *fs, int inumber, 
    int fileBlockIndex) {

  // Bounds check
  int max_inode_number = fs->superblock.s_isize * 16;
  if (inumber < 1 || inumber > max_inode_number) {
    printf("ERROR: invalid inumber for this disk; ");
    printf("must be between 1 and %d, inclusive\n", max_inode_number);
    return;
  }

  // Read in the inode for error checking purposes
  struct inode in;
  int result = inode_iget(fs, inumber, &in);
  if (result == -1) {
    printf("inode_iget(%d) returned -1\n", inumber);
    return;
  }

  if ((in.i_mode & IALLOC) == 0) {
    printf("ERROR: this inode is marked as free, cannot access its blocks.\n");
    return;
  }

  // Read the block
  char buf[DISKIMG_SECTOR_SIZE];
  result = file_getblock(fs, inumber, fileBlockIndex, buf);
  printf("file_getblock(inumber = %d, fileBlockIndex = %d) returned %d\n", 
    inumber, fileBlockIndex, result);
  if (result == -1) {
    return;
  }

  /* Print the file block in hex, 16 digits per line, padded to 2 digits
   * (padding from StackOverflow post 13275258, hh from 3555791)
   */
  printf("File block contents:\n\n");
  for (int i = 0; i < result; i++) {
    if (i % 16 == 0 && i != 0) printf("\n");
    printf("%02hhx ", buf[i]);
  }
  printf("\n");
}

/* This function tests the directory_findname function; it error checks for if
 * the specified inumber is invalid for this filesystem, if this inode
 * is free (and therefore we cannot read its blocks), if this inode
 * is not a directory, or if inode_iget returned -1, and otherwise prints 
 * the contents of the direntv6 that was read in.
 */
static void test_directory_findname(const struct unixfilesystem *fs, 
    int dirinumber, const char *name) {

  // Bounds check
  int max_inode_number = fs->superblock.s_isize * 16;
  if (dirinumber < 1 || dirinumber > max_inode_number) {
    printf("ERROR: invalid inumber for this disk; ");
    printf("must be between 1 and %d, inclusive\n", max_inode_number);
    return;
  }

  // Read in the inode for error checking purposes
  struct inode in;
  int result = inode_iget(fs, dirinumber, &in);
  if (result == -1) {
    printf("inode_iget(%d) returned -1\n", dirinumber);
    return;
  }

  // Ensure that this inode is allocated and a directory
  if ((in.i_mode & IALLOC) == 0) {
    printf("ERROR: this inode is marked as free, cannot access its blocks.\n");
    return;
  } else if ((in.i_mode & IFMT) != IFDIR) {
    printf("ERROR: this inode does not represent a directory.\n");
    return;
  }

  // Read the dirent
  struct direntv6 entry;
  result = directory_findname(fs, name, dirinumber, &entry);
  if (result == -1) {
    printf("directory_findname(dirinumber = %d, name = '%s') returned -1\n", 
      dirinumber, name);
  } else {
    printf("direntv6 {\n");
    printf("\td_inumber = %d,\n", entry.d_inumber);
    printf("\td_name = '%.14s'\n", entry.d_name);
    printf("}\n");
  }
}

/* This function tests the pathname_lookup function; it error checks for if
 * the specified path is empty or not an absolute path beginning with "/", and
 * otherwise prints the return value of pathname_lookup.
 */
static void test_pathname_lookup(const struct unixfilesystem *fs, 
    const char *pathname) {

  // Must be absolute path
  if (strlen(pathname) == 0 || pathname[0] != '/') {
    printf("ERROR: pathname_lookup requires absolute paths ");
    printf("(must begin with /)\n");
    return;
  }

  int result = pathname_lookup(fs, pathname);
  printf("pathname_lookup(\"%s\") returned %d\n", pathname, result);
}

static void printUsage(char *progname) {
  printf("Usage: %s <diskimagePath> <function> <arg1>...<argn>\n", progname);
  printf("<diskimagePath> is the path to a disk image file.\n");
  printf("<function> is one of the assignment functions, e.g. inode_indexlookup or file_getblock.\n");
  printf("<argX> is an arg to pass to that function.  Here are examples:\n");
  printf("\tinode_iget: specify the inode number to test with\n");
  printf("\tinode_indexlookup: specify the inode number followed by the fileBlockIndex to test with\n");
  printf("\tfile_getblock: specify the inode number followed by the fileBlockIndex to test with\n");
  printf("\tdirectory_findname: specify the directory inumber followed by the entry name\n");
  printf("\tpathname_lookup: specify the absolute path\n");
}

int main(int argc, char *argv[]) {
  // disk function params
  if (argc < 4) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }

  // First, load the specified disk image
  char *diskpath = argv[1];
  int fd = diskimg_open(diskpath, 1);
  if (fd < 0) {
    fprintf(stderr, "Can't open diskimagePath %s\n", diskpath);
    return EXIT_FAILURE;
  }
  struct unixfilesystem *fs = unixfilesystem_init(fd);
  if (!fs) {
    fprintf(stderr, "Failed to initialize unix filesystem\n");
    return EXIT_FAILURE;
  }

  // Next, identify which function is being tested
  if (strcmp(argv[2], "inode_iget") == 0) {
    test_inode_iget(fs, atoi(argv[3]));
  } else if (strcmp(argv[2], "inode_indexlookup") == 0) {
    if (argc != 5) {
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }
    test_inode_indexlookup(fs, atoi(argv[3]), atoi(argv[4]));
  } else if (strcmp(argv[2], "file_getblock") == 0) {
    if (argc != 5) {
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }
    test_file_getblock(fs, atoi(argv[3]), atoi(argv[4]));
  } else if (strcmp(argv[2], "directory_findname") == 0) {
    if (argc != 5) {
      printUsage(argv[0]);
      return EXIT_FAILURE;
    }
    test_directory_findname(fs, atoi(argv[3]), argv[4]);
  } else if (strcmp(argv[2], "pathname_lookup") == 0) {
    test_pathname_lookup(fs, argv[3]);
  } else {
    printf("ERROR: unknown function '%s'.\n", argv[2]);
    return EXIT_FAILURE;
  }

  // Close the disk image when we're done
  int err = diskimg_close(fd);
  if (err < 0) fprintf(stderr, "Error closing %s\n", argv[1]);
  free(fs);
  return 0;
}
