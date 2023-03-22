#include "virtualmemoryregion.hh"

using namespace std;


void VirtualMemoryRegion::handle_fault(char *fault_addr) {
    // TODO: add your code below to handle a page fault
    VPage virtual_page = fault_addr - ((unsigned long)fault_addr % get_page_size());
    info page_info;
    if (page_map.find(virtual_page) == page_map.end()) { // not found on map, create new page
        PPage physical_page = physical_memory_ -> get_new_ppage(virtual_page, this);
        map(virtual_page, physical_page, PROT_READ);
        page_info.pp = physical_page;
        page_info.status = PROT_READ;
        page_info.is_dirty = false;
        page_map[virtual_page] = page_info;
        if (disk_ -> is_page_stored_on_disk(virtual_page)) {
            disk_ -> load_page_from_disk(virtual_page, physical_page);
        }
    }

    else { // found mapping, change protection
        page_info = page_map[virtual_page];
        if (page_info.status == PROT_NONE) { // if was swept
            map(virtual_page, page_info.pp, PROT_READ);
        }
        else {
            map(virtual_page, page_info.pp, PROT_READ | PROT_WRITE);
            page_info.is_dirty = true;
            page_map[virtual_page] = page_info;
        }
    }

}

VirtualMemoryRegion::~VirtualMemoryRegion() {
    // TODO: unmap all virtual pages and free any used physical pages
    for (auto &pair : page_map) {
        unmap(pair.first);
        physical_memory_ -> page_free(pair.second.pp);
        // page_map.erase(pair.first); ??? why don't need it? because it comes with constructor?
    }
}

void VirtualMemoryRegion::clock_sweep(VPage vp) {
    // TODO: your code here
    info page_info = page_map[vp];
    page_info.status = PROT_NONE;
    page_map[vp] = page_info;
    map(vp, page_info.pp, PROT_NONE);
}

bool VirtualMemoryRegion::clock_should_remove(VPage vp) {
    // TODO: your code here (remove this line)
    info page_info = page_map[vp];
    if (page_info.status == PROT_NONE) return true;
    return false;
}

void VirtualMemoryRegion::clock_remove(VPage vp) {
    // TODO: your code here
    info page_info = page_map[vp];
    if (page_info.is_dirty) {
        disk_ -> store_page_to_disk(vp, page_info.pp);
    }
    unmap(vp);
    physical_memory_ -> page_free(page_info.pp);
    page_map.erase(vp);
}
