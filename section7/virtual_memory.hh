#pragma once

#include <map>
#include <cstddef>
#include "diskregion.hh"

/* This file defines two types that simulate how the "demand paging" approach
 * to virtual memory works.
 *
 * VirtualMemory is a variable type that represents a fake "virtual address space"
 * of arbitrary size, and backed by a specified number of physical pages.
 *
 * VirtualMemory can give you a pointer to the start of the memory region
 * use it just as you would a char * (dereference, pointer arithmetic).  
 * 
 * The pointer is of type VirtualPointer, but it behaves just like a pointer.  You
 * can dereference it, do pointer arithmetic, and print it out and it will print
 * out the address it stores.
 *
 *
 * If the client accesses a page we have mapped already, we just translate the
 * address and return it.  If they access an unmapped page, we must map it first
 * and then translate.  
 *
 * The internal page map stores only present pages - and it should never have
 * more than nppages entries.  So if we need to make a new mapping but 
 * don't have room for more entries, we must swap one to disk first.  And
 * when we make a new mapping, we must check disk to see if this page was
 * previously swapped out.
 */

const int PAGE_SIZE = 4096;

class VirtualPointer;

/* A variable type that simulates a virtual address space managed by the OS using
 * the "demand paging" approach.
 */
class VirtualMemory {
public:

  /* Initialize it with the specified number of physical pages.  The page
   * map should have at most this many entries.
   */
    VirtualMemory(size_t nppages) : nphysical_pages(nppages), disk() { disk.enable_logging(); };

  /* Returns a pointer to the start of the virtual address space.
   * A VirtualPointer behaves just like a regular char * pointer in that we can
   * dereference it, write to that location, print it out, 
   * or do pointer arithmetic with it.
   */
  VirtualPointer get_start_ptr();

  // This line allows a VirtualPointer to access our private fields
  friend VirtualPointer;

  // Clean up and free memory in our destructor
  ~VirtualMemory();
private:
  /* The number of physical pages that can be used at any given time.
   * page_map should have at most this many entries.
   */
  size_t nphysical_pages;

  /* The page map is a map (C++ type map) from virtual pages
   * to physical pages.  They are both pointers to the start of pages.
   */
  std::map<char *, char *> page_map;

  // Returns a randomly-selected key from the page_map.
  char *get_random_vpage();

  /* This function performs the translation from a "virtual address"
   * to a "physical address".  It should look up the page info for this 
   * virtual address in the page map, and if it's present, return the 
   * actual physical address this is mapped to.
   * If it's not present, we need to map it, possibly by swapping
   * another page to disk.
   */
  char *translate(char *virtual_addr);

  // The disk swap space where we can store kicked-out pages and load pages
  DiskRegion disk;
};


/* A variable type that simulates a single virtual address that uses 
 * the "multiple segments" approach.  A VirtualPointer behaves just like a char *
 * (you can dereference it to get/set a char, print it out, do pointer arithmetic
 * with it), and internally it stores its address.  When you print
 * it out it prints out its address.
 *
 * You can ignore the implementation of this.
 */
class VirtualPointer {
public:
  /* This "overloads" the dereference and addition operators and allows us
   * to run our own code when someone dereferences a VirtualPointer or adds
   * a number to a VirtualPointer.
   */
  char& operator*();
  VirtualPointer operator+(int op) const;

  /* This "overloads" the << operator to allow users to print a VirtualPointer
   * to a stream like cout.  It will print its address.
   */
  friend std::ostream& operator<<(std::ostream& os, const VirtualPointer& p);

  // This line allows VirtualMemory to access our private fields
  friend VirtualMemory;

private:
  /* Initializes a new VirtualPointer to point to somewhere within the given
   * virtual address space.  This constructor is private, so only people who
   * have permission (like VirtualMemory) can create new VirtualPointers.
   */
  VirtualPointer(VirtualMemory *vm, char *ptr) 
    : addr(ptr), vm(vm) {};

  // Internal address
  char *addr;

  // We need a reference to the original VirtualMemory object to call translate()
  VirtualMemory *vm;
};
