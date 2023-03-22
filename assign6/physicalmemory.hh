#pragma once

#include <stddef.h>
#include "vm.hh"

class VirtualMemoryRegion;

/* PhysicalMemory represents a collection of physical memory pages on the
 * system (assumed to be contiguous).  It's initialized with a fixed number
 * of available physical pages and you can request a new physical page 
 * from it.  If one is available, it will just give you a new one; otherwise,
 * it runs the clock algorithm to choose an existing physical page 
 * to throw out and reuse.
 */
class PhysicalMemory : public PhysMem {
public:
    /* Initialize physical memory with the given number of physical pages
     * in the unallocated pool.
     */
    PhysicalMemory(std::size_t npages);

    /* Request a new physical page for the given virtual page in the given
     * virtual memory region.  If a physical page is free, this just returns
     * an available physical page.  Otherwise, it runs the clock algorithm to 
     * choose an in-use physical page to throw out/reuse.  Whoever calls this
     * must specify mapped_page, which is the virtual page that will be mapped
     * to the physical page we return, and which VirtualMemoryRegion is doing
     * that mapping (i.e. which region is the one using the physical page
     * that we return).
     */
    PPage get_new_ppage(VPage mapped_page, VirtualMemoryRegion *owner);

    /* This already-implemented method returns a physical page to the 
     * unallocated pool.
     */
    void page_free(PPage p) { PhysMem::page_free(p); }

private:
    /* This already-implemented method returns the starting address of 
     * the block of physical memory used for the physical pages (all pages are 
     * contiguous).
     */
    PPage pool_base() { return PhysMem::pool_base(); }

    /* This already-implemented method returns the number of pages in the
     * unallocated pool.
     */ 
    std::size_t nfree() { return PhysMem::nfree(); }
    
    /* This already-implemented method gets a physical page from the 
     * unallocated pool.  Returns nullptr if no pages are available.
     */
    PPage page_alloc() { return PhysMem::page_alloc(); }

    // Allows the test harness to access private methods for testing purposes
    friend class VMScript;

    // Add any additional private members here
    typedef struct {
        VPage vp;
        VirtualMemoryRegion *vmr;
    } vinfo;
    std::vector<vinfo> vinfo_vec;
    PPage starting_ppage;

};
