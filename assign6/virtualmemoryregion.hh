#pragma once
#include <stddef.h>
#include "vm.hh"
#include "physicalmemory.hh"
#include "diskregion.hh"
#include <unordered_map>

class VirtualMemoryRegion : public VMRegion {
public:
    /* This is implemented for you - initializes a VirtualMemoryRegion of the 
     * given size, and use the specified PhysicalMemory object to 
     * get physical pages as needed, and the specified disk object to 
     * read stored contents from disk and store memory swapped to disk.  
     * This implementation sets up handle handle_fault to be called 
     * whenever a page fault happens, and stores references to physmem and disk
     * in our instance variables physical_memory_ and disk_, respectively.
     */
    VirtualMemoryRegion(std::size_t nbytes, PhysicalMemory *physmem, 
        DiskRegion *disk) 
                : VMRegion(nbytes, [this](char *p) { handle_fault(p); }),
                  physical_memory_(physmem), disk_(disk) {}

    /* Cleans up the virtual memory region by unmapping any mapped
     * virtual pages and freeing all used physical pages.
     */
    ~VirtualMemoryRegion();

    /* Indicates that the clock algorithm ran and "swept over" this page;
     * update the internal state to indicate that this page was swept over.
     */
    void clock_sweep(VPage vp);

    /* Should return true if the page was not accessed since the last time
     * the clock hand swept over this page, or false otherwise.
     */
    bool clock_should_remove(VPage vp);

    /* Indicates that the clock algorithm kicked this page out and is reusing
     * its physical page for something else; update the internal state to
     * indicate that this page was kicked out.
     */
    void clock_remove(VPage vp);

private:
    /* This method is implemented for you - it officially records the mapping
     * from the given virtual page to the given physical page with the given
     * protections.
     */
    void map(VPage va, PPage pa, Prot prot) { VMRegion::map(va, pa, prot); }

    /* This method is implemented for you - it officially unrecords the
     * mapping for the given virtual page.
     */
    void unmap(VPage va) { VMRegion::unmap(va); }

    /* This function is called whenever a page fault occurs; it is passed
     * the faulting virtual address as a parameter.
     */
    void handle_fault(char *fault_addr);

    /* This is the object we can call get_new_ppage and page_free on to
     * get a new physical page and free a physical page, respectively.
     */
    PhysicalMemory *physical_memory_;

    /* This is the object we can call is_page_stored_on_disk, 
     * load_page_from_disk and store_page_to_disk on to load existing page 
     * data from disk and swap page data to disk.
     */
    DiskRegion *disk_;

    //TODO: add any additional private members below
    typedef struct {
        PPage pp;
        Prot status;
        bool is_dirty;
    } info;
    std:: unordered_map<VPage, info> page_map;
};
