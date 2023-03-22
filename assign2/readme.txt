File: readme.txt
Author: Jie Zhu
----------------------
1) This log entry represents that creating a directory entry dir1 with inode 101

2) Block 1027 is marked as allocated to be used to store its directory entries

3) There is nothing in the mnt folder. This might be because fsck doesn't help prevent loss information, it just help restore consistency. And sometimes even restore consistency, 
    file system may stil be unuseable.

4) After apply_soln, there are two folders dir1 and dir2. dir1 folder has hello.txt file and dir2 has empty file named from 1 to 96. 
   Compared to fsck, apply_soln does recover some loss data（metadata). However, there is still one file missing in dir2 which is 97.
   The contents of hello.txt are not recoverable because only metadata is recovered from the log not the file data.

5)  a. The user program can use readSector to function read the root (inumber 1, the first inode) in sector 2. By checking large or small size,
        the user program can iterate the block correctly and get the file names from directory entries.
    b. The user program can look at the dirent (directory entry) in the log file to know the file names.

6) I may not put my personal work file for a long time on the myth machine if there is a permission bug. Because someone may get my personal file information (such as homework) directly

7) In favor of requiring minimum support periods for os, users can have time to complete the system transition process and avoid potential loss due to the retirement of the system.
   However, providing minimum support periods for old system will also increase the wordload for system developers that may affect the integrity and support of the new system.

