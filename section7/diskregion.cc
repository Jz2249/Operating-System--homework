#include "diskregion.hh"
#include <iostream>
#include <cstring>

using namespace std;

bool DiskRegion::is_page_stored_on_disk(const VPage vpage) {
    return backing_store.find(vpage) != backing_store.end();
}

void DiskRegion::load_page_from_disk(const VPage vpage, PPage dst) {
    load_data(vpage, dst, true);
}

// Same as load_page_from_disk except we can optionally not count towards stats
void DiskRegion::load_data(const VPage vpage, void *dst, bool record_stats) {
    if (dst == nullptr) {
        cerr << "Null dst passed to DiskRegion::load" << endl;
        abort();
    }

    // If there's no record for this virtual page, error
    if (!is_page_stored_on_disk(vpage)) {
        cerr << "trying to load non-existent data from disk for vpage " 
             << hex << vpage << endl;
        abort();
    }

    if (logging_enabled) cout << " [DiskRegion: loading page " << (void *)vpage << "] ";

    // Otherwise, read it in and remove it from the backing store
    memcpy(dst, backing_store[vpage], get_page_size());
    free(backing_store[vpage]);
    backing_store.erase(vpage);
    if (record_stats) pages_read++;
}

void DiskRegion::store_page_to_disk(const VPage vpage, const PPage src) {
    store_data(vpage, src, true);
}

// Same as store_page_to_disk except we can optionally not count towards stats
void DiskRegion::store_data(const VPage vpage, const void *src, 
    bool record_stats) {

    if (src == nullptr) {
        cerr << "Null src passed to DiskRegion::store" << endl;
        abort();
    }

    /* If there's something already stored for this page, it means we are
     * writing something to disk before we read its existing contents, which
     * isn't what we want (every time something is loaded, it's removed from
     * backing_store).
     */
    if (is_page_stored_on_disk(vpage)) {
        cerr << "BACKING STORE ERROR: request to store page already stored - " 
             << hex << vpage << endl;
        abort();
    }

    if (logging_enabled) cout << " [DiskRegion: storing page " << (void *)vpage << "] ";

    // Otherwise, store it in our map
    backing_store[vpage] = malloc(get_page_size());
    memcpy(backing_store[vpage], src, get_page_size());
    if (record_stats) pages_written++;
}
