/**
 * File: test.cc
 * ---------------
 * This program creates a simulated virtual address space and writes/accesses
 * various memory addresses.
 */

#include "virtual_memory.hh"
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  // Initialize with just 1 physical page
  VirtualMemory v_mem(1);

  /* Get a virtual pointer to the start of the address space.
   * A VirtualPointer can be used just like a 
   * regular char * pointer.
   */
  VirtualPointer ptr = v_mem.get_start_ptr();

  /* Write to the first byte of the first 5 pages
   * Only one of these can really be mapped at a time
   */
  for (int i = 0; i < 5; i++) {
    VirtualPointer newPtr = ptr + (i * PAGE_SIZE);
    *newPtr = 'a' + i;
    cout << "Data at virtual address " << newPtr 
         << " is: " << *newPtr << endl;
  }

  /* Now read the first byte of the first 5 pages
   * This will require swapping to/from disk
   */
  for (int i = 0; i < 5; i++) {
    VirtualPointer newPtr = ptr + (i * PAGE_SIZE);
    cout << "Data at virtual address " << newPtr 
         << " is: " << *newPtr << endl;
  }

  return 0;
}
