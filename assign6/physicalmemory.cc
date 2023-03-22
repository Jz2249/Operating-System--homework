#include "physicalmemory.hh"
#include "virtualmemoryregion.hh"

using namespace std;


PhysicalMemory::PhysicalMemory(size_t npages) : PhysMem(npages) {
    // TODO: initialize any instance variables here
    vinfo_vec.resize(npages);
    starting_ppage = pool_base();
}

PPage PhysicalMemory::get_new_ppage(VPage mapped_page, 
    VirtualMemoryRegion *owner) {

    /* TODO: change this so that it checks for available pages,
     * and if no physical pages are free, throws out a page using the clock
     * algorithm.
     */
    vinfo info;
    PPage pp;
    int index;
    if (nfree() > 0) {
        pp = page_alloc();
        index = (pp - pool_base()) / get_page_size();
    }
    else { // none free memory in pool, call clock algo
        index = (starting_ppage - pool_base()) / get_page_size();
        while (true) {
            info = vinfo_vec[index];
            int next_index = (index + 1) % vinfo_vec.size(); 
            if (info.vmr -> clock_should_remove(info.vp)) {
                info.vmr -> clock_remove(info.vp);
                pp = page_alloc();
                starting_ppage = next_index * get_page_size() + pool_base(); // update next hand
                break;
            }
            else {
                info.vmr -> clock_sweep(info.vp);
                index = next_index; // next index
            }
        }
    }
    info.vp = mapped_page;
    info.vmr = owner;
    vinfo_vec[index] = info;
    return pp;
}
