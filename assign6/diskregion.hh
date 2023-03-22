#pragma once

#include "vm.hh"
#include <map>

/* DiskRegion represents a region of disk from where a process can read 
 * contents into memory or swap memory contents to disk.  It can store 
 a virtual page to disk (e.g. if it's kicked out) to read back later, 
 * and it can load a virtual page from disk (e.g. if you stored it previously, 
 * or if it's being populated with existing contents from the executable).
 */ 
class DiskRegion {
public:
    /* Returns true if there is stored data for the specified virtual page
     * (e.g. this virtual page should be pre-populated with data from the
     * executable, or it was previously swapped to disk).  Otherwise
     * returns false.
     */
    bool is_page_stored_on_disk(const VPage vpage);

    /* Copy in stored data for this virtual page from disk to the specified
     * physical page.  If there's no data for this virtual page, throws
     * an error - make sure to call is_page_stored_on_disk first to confirm
     * there is stored data.  vpage is assumed to map to dst, but there are two
     * parameters because the vpage may not be writeable (eg. may be
     * PROT_READ).  Removes the stored data from disk.
     */
    void load_page_from_disk(const VPage vpage, PPage dst);

    /* Store the specified contents to swap space on disk.
     * The DiskRegion will remember the contents as being associated with
     * the specified virtual page address, so when you load the same virtual 
     * page in later, it will give back the data stored via this method.  vpage
     * is assumed to map to src, but there are two parameters because vpage
     * may not be readable (e.g. may be PROT_NONE).
     */
    void store_page_to_disk(const VPage vpage, const PPage src);

    /* --- Ignore code below this line --- */

    // Utility functions for logging / testing purposes
    std::size_t get_pages_read() { return pages_read; }
    std::size_t get_pages_written() { return pages_written; }
    std::size_t get_backing_store_size() { return backing_store.size(); }

    // Utility functions for planting test data/checking it
    void store_data(const VPage vpage, const void *src, 
        bool record_stats = false);
    void load_data(const VPage vpage, void *dst, bool record_stats = false);

    void enable_logging() { logging_enabled = true; }
private:
    /* Store all the swapped/"disk" contents in this map, which maps
     * virtual pages to heap-allocated chunks of data.
     */
    std::map<VPage, void *> backing_store;

    std::size_t pages_read = 0;
    std::size_t pages_written = 0;

    bool logging_enabled = false;
};
