/* CS111 Section 7
 * Written by Nick Troccoli, with modifications by Parthiv Krishna
 */

#include "virtual_memory.hh"
#include <iostream>

using namespace std;

char *VirtualMemory::translate(char *virtual_addr) {
  // nphysical_pages is the most entries we can have in our map at once
  assert(page_map.size() <= nphysical_pages);

  /* The offset is the part of the address 
   * not divisible by PAGE_SIZE
   */
  size_t offset = (unsigned long)virtual_addr % PAGE_SIZE;
  // The start of the page is just offset 0
  char *vpage_start = virtual_addr - offset;

  // You implement the rest (replace these lines below;
  // these are just to avoid compiler warnings)

  if (page_map.find(vpage_start) == page_map.end()) {
    char *ppage = nullptr;
    if(page_map.size() == nphysical_pages) { // no more free pages
      char *rpage = get_random_vpage();
      disk.store_page_to_disk(rpage, page_map[rpage]);
      ppage = page_map[rpage];
      page_map.erase(rpage);
    }

    else {
      ppage = (char*)malloc(PAGE_SIZE);
    }

    page_map[vpage_start] = ppage;

    if (disk.is_page_stored_on_disk(vpage_start)) {
      disk.load_page_from_disk(vpage_start, ppage);
    }
  }


  return page_map[vpage_start] + offset;
}

/* ----- You can ignore everything below this line ------ */

char *VirtualMemory::get_random_vpage() {
  // From StackOverflow question 15425442
  auto it = page_map.begin();
  advance(it, rand() % page_map.size());
  return it->first;
}

VirtualMemory::~VirtualMemory() {
  // To be good memory citizens, we should free the heap memory in each PageInfo
  for (const auto& [key, value] : page_map) {
    free(value);
  }
}

VirtualPointer VirtualMemory::get_start_ptr() {
  /* Make a new virtual pointer referring to this virtual memory space,
   * pointing to vpage 0, offset 0.
   */
  return VirtualPointer(this, 0);
}

char& VirtualPointer::operator*() {
  // If someone dereferences us, return a reference to the physical memory location
  return *(vm->translate(addr));
}

VirtualPointer VirtualPointer::operator+(int op) const {
  // If someone adds to us, make a new pointer adding the specified amount
  return VirtualPointer(vm, addr + op);
}

ostream& operator<<(ostream& os, const VirtualPointer& p) {
  // These operators print the number in hex
  os << (void *)p.addr;
  return os;
}
