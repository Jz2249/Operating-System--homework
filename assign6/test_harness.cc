/*
 * Files: test_harness.cc
 * ---------------------
 * Reads/runs text-based script files containing a sequence of
 * virtual memory events.
 *
 * Based on a Heap Allocator test harness for CS107 written by Julie Zelenski.
 *
 * Run by specifying one or more script files:
 *
 *      ./test_harness script1.txt script2.txt script3.txt
 */

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "virtualmemoryregion.hh"
#include "physicalmemory.hh"
#include "diskregion.hh"
#include "vm.hh"

using namespace std;

/* A type representing a single virtual memory script file that can be
 * executed on the VirtualMemoryRegion / PhysicalMemory classes.  Initialize
 * it with the path to the script file, call parse() to read it in,
 * and call run() to run it.
 *
 * The format of a script file is expected to be as follows:
 *
 * First line: one number, the number of physical pages that should be 
 * available in the pool for this script.
 *
 * Subsequent lines: one of the following:
 * - INIT ID X
 *      ID is a unique number IDing this virtual memory region.  
 *      X is the number of virtual pages that should be in its 
 *      virtual address space.
 *
 * - READ ID X
 *      ID is the virtual memory region to read from.
 *      X is virtual page number (0-indexed) to try to read from.
 *      If the page was previously stored, print the contents signature.
 *
 * - WRITE ID X
 *      ID is the virtual memory region to write to.
 *      X is virtual page number (0-indexed) to try to write to.
 *
 * - STORE ID X
 *      ID is the virtual memory region to store data for.
 *      X is virtual page number (0-indexed) to pre-store data for on disk.
 *      If the page is read later, it will print the contents signature.
 *      Stores abitrary but unique data for this page.
 *
 * - SWEEP ID X
 *      ID is the virtual memory region to refer to.
 *      X is virtual page number (0-indexed) to call clock_sweep on.
 *
 * - REMOVE ID X
 *      ID is the virtual memory region to refer to.
 *      X is virtual page number (0-indexed) to call clock_remove on.
 *
 * - CHECK_SHOULD_REMOVE ID X
 *      ID is the virtual memory region to refer to.
 *      X is virtual page number (0-indexed) to call clock_should_remove on.
 *
 * - CHECK_CLEANUP
 *      Check that all vpages are unmapped and ppages freed on destruction.
 *      Can appear anywhere in the script (not required to be last line).
 *
 * Empty lines or lines beginning with # are treated as comments and ignored
 *
 * Sample script with 1 physical page, 1 VA space with 2 pages, that reads
 * a pre-existing page 0, and writes to page 1 before checking all is cleaned
 * up properly:
 *
 * 1
 * INIT 1 2
 * STORE 1 0
 * READ 1 0
 * WRITE 1 1
 * CHECK_CLEANUP
 */
class VMScript {
public:
    // Initializes a script with the given path, but doesn't read/run it.
    VMScript(const string& path) : script_path(path) { 
        // Parse out name
        size_t last_slash = script_path.find_last_of('/');
        if (last_slash == string::npos) name = script_path;
        else name = script_path.substr(last_slash + 1);
    }

    ~VMScript() {
        // Free any memory allocated for VirtualMemoryRegions or DiskRegions.
        for (auto it = regions.begin(); it != regions.end(); ++it) {
            if ((it->second).vmregion) delete (it->second).vmregion;
            if ((it->second).disk) delete (it->second).disk;
        }

        // Free any memory allocated for PhysicalMemory.
        if (physmem) delete physmem;
    }

    // Return the script filename (path contents after last /)
    string get_name() { return name; }

    // Reads in the script file and stores its info to run
    bool parse();

    /* Executes the script file on a VirtualMemoryRegion and
     * PhysicalMemory instance.
     */
    void run();
private:
    /* Parses a single line consisting of the given tokens (assumed to
     * be non-empty).  If any errors are found, prints an error with the
     * given line number and returns false.  Assumed to be any line other
     * than the first line in the script.
     */
    bool parse_line(const vector<string>& tokens, int linenum);

    // Print out information about physical memory (pages used / total pool)
    void print_physmem_summary(PhysicalMemory *physmem);

    // Each line must be one of these types of events (or CHECK_CLEANUP/INIT)
    enum EventType {
        READ = 1,
        WRITE,
        STORE,
        SWEEP,
        REMOVE,
        CHECK_SHOULD_REMOVE
    };

    // Contains info for a single script line
    typedef struct {
        EventType e;
        int vm_id;      // ID of VirtualMemoryRegion
        size_t vpage_num; // optional for some event types
    } ScriptLine;

    // Ordered list of info about each script line
    vector<ScriptLine> script;

    // Path to script file
    string script_path;

    // Component of path after last /
    string name;

    // Shared physical memory for all ops
    PhysicalMemory *physmem = nullptr;

    /* Whether we should do error checking for the destructor of 
     * VirtualMemoryRegion to ensure all pages are unmapped/freed.
     * true if CHECK_CLEANUP is in the script, or false otherwise.
     */
    bool check_cleanup = false;

    /* Map from ID -> vmregion info so we can have multiple
     * VirtualMemoryRegions per script, referred to by unique ID.
     */
    typedef struct {
        VirtualMemoryRegion *vmregion = nullptr;
        DiskRegion *disk = nullptr;
        set<size_t> stored_pages; // pages pre-stored in disk
    } VMInfo;
    unordered_map<int, VMInfo> regions;
};

bool VMScript::parse_line(const vector<string>& tokens, int linenum) {
    assert(tokens.size() > 0);
    
    ScriptLine line_info;

    // Parse out event type
    if (tokens[0] == "INIT") {
        if (tokens.size() != 3) {
            cerr << "Invalid script line " << linenum 
                 << " - INIT requires ID and #vpages" << endl;
            return false;
        }

        // Parse out ID# and #vpages
        size_t id;
        size_t nvpages;
        try {
            id = stoi(tokens[1]);
            nvpages = stoi(tokens[2]);            
        } catch (std::invalid_argument const& e) {
            cerr << "Invalid args to INIT on line " << linenum << endl;
            return false;
        }

        VMInfo info;
        info.disk = new DiskRegion();
        info.vmregion = new VirtualMemoryRegion(nvpages * get_page_size(),
            physmem, info.disk);
        info.vmregion->vm_id = id;
        regions[id] = info;
        return true;
    } else if (tokens[0] == "READ") line_info.e = READ;
    else if (tokens[0] == "WRITE") line_info.e = WRITE;
    else if (tokens[0] == "STORE") line_info.e = STORE;
    else if (tokens[0] == "SWEEP") line_info.e = SWEEP;
    else if (tokens[0] == "REMOVE") line_info.e = REMOVE;
    else if (tokens[0] == "CHECK_SHOULD_REMOVE") {
        line_info.e = CHECK_SHOULD_REMOVE;
    } else if (tokens[0] == "CHECK_CLEANUP") { 
        check_cleanup = true;

        if (tokens.size() > 1) {
            cerr << "Invalid script line " << linenum 
                 << " - CHECK_CLEANUP doesn't accept params" << endl;
            return false;
        }
        return true; 
    } else {
        cerr << "Invalid script command '" << tokens[0] << "' on line " 
             << linenum << endl;
        return false;
    }

    // Check for ID and vpage#
    if (tokens.size() != 3) {
        cerr << "Invalid script line " << linenum 
             << " - must specify ID# and vpage#" << endl;
        return false;
    }

    // Parse out ID# and vpage#
    try {
        line_info.vm_id = stoi(tokens[1]);
        line_info.vpage_num = stoi(tokens[2]);
    } catch (std::invalid_argument const& e) {
        cerr << "Invalid args on line " << linenum << endl;
        return false;
    }

    if (regions.find(line_info.vm_id) == regions.end()) {
        cerr << "Invalid ID# on line " << linenum
             << " (must INIT first)" << endl;
        return false;
    }

    // The number of vpages in this region
    size_t n_vpages = regions[line_info.vm_id].vmregion->get_size() 
                        / get_page_size();

    if (line_info.vpage_num >= n_vpages) {
        cerr << "Invalid vpage# on line " << linenum 
             << " (must be < " << n_vpages << ") " << endl;
        return false;
    }

    script.push_back(line_info);
    return true;
}

bool VMScript::parse() {    
    ifstream file(script_path);
    if (!file.is_open()) {
        cerr << "Unable to open file." << endl;
        return false;
    }

    // Parse each line one at a time
    string line;
    int linenum = 1;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        // Tokenize the line
        istringstream ss(line);
        string token;
        vector<string> tokens;
        while(getline(ss, token, ' ')) {
            tokens.push_back(token);
        }

        if (linenum == 1) {
            // First line is special - specifies physical address space size
            if (tokens.size() != 1) {
                cerr << "Invalid first line - must be NPPAGES" << endl;
                return false;
            }

            try {
                size_t num_total_ppages = stoi(tokens[0]);     
                physmem = new PhysicalMemory(num_total_ppages);           
            } catch (std::invalid_argument const& e) {
                cerr << "Invalid # ppages" << endl;
                return false;
            }
        } else {
            // Other lines
            bool result = parse_line(tokens, linenum);
            if (!result) return false;
        }

        linenum++;
    }
    file.close();

    // Update error checking based on whether CHECK_CLEANUP was included
    if (!check_cleanup) {
        physmem->check_used_pages_on_exit = false;
        for (auto it = regions.begin(); it != regions.end(); ++it) {
            (it->second).vmregion->check_mapped_pages_on_exit = false;
        }
    }

    return true;
}

// Reads a given memory location.
static int dummy;
void read_byte(char *addr) {
    // This update is just intended to ensure that the compiler can't
    // optimize out the actual memory read.
    dummy += *addr;
}

// Prints info about physical pages used / total
void VMScript::print_physmem_summary(PhysicalMemory *physmem) {
    cout << "Physical memory: " << physmem->npages() - physmem->nfree() << "/" 
         << physmem->npages() << " pages allocated" << endl;
}

// Prints info about reads/writes to the disk
void print_diskregion_summary(DiskRegion *disk, int vm_id) {
    cout << "Disk region #" << vm_id << ": " << disk->get_pages_read() 
         << " pages read, " << disk->get_pages_written() 
         << " pages written." << endl;
}

//! Overwrites all of the bytes of a given page with distinct
//! identifying information including a label (which shouldn't be more
//! than 10-20 chars.) and the index of this page within the file
//! (page_index). The page will also contain an internal checksum.
static const char *sample_data = "00000111112222233333444445555566666777778888899999";
void fill_page(char *page, const char *label, int page_index)
{
    size_t msg_length = snprintf(page, page_size, "%s, page %d", label,
            page_index);

    // Fill in the rest of the page with character data.
    const char *src = sample_data;
    for (size_t i = msg_length+1; i < page_size; ++i) {
        page[i] = *src;
        src++;
        if (*src == '\0') {
            src = sample_data;
        }
    }

    // Compute a checksum such that all of the 32-bit words of the page
    // add up to 0, and place it in the last word of the page.
    int32_t *words = reinterpret_cast<int32_t *>(page);
    size_t num_words = page_size/sizeof(int32_t);
    int32_t sum = 0;
    for (size_t i = 0; i < (num_words-1); ++i) {
        sum += words[i];
    }
    words[num_words-1] = -sum;
}

//! Given a page of memory that was filled by fill_page, this function
//! returns a printable string describing the page, including the label
//! and page_index passed to fill_page, plus the page's checksum, which
//! should be 0.
std::string page_signature(char *page)
{
    // Compute the checksum of the page.
    int32_t *words = reinterpret_cast<int32_t *>(page);
    size_t num_words = page_size/sizeof(int32_t);
    int32_t sum = 0;
    for (size_t i = 0; i < num_words; ++i) {
        sum += words[i];
    }

    // Make sure the page contains an initial null-terminated string
    // that isn't very long.
    size_t length;
    for (length = 0; length < 100; length++) {
        if (page[length] == '\0') {
            break;
        }
    }

    for (size_t i = 0; i < length; ++i) {
        if (page[i] & 0x80) {
            // Garbage character.
            return "!!garbage!!, checksum " + std::to_string(sum);
        }
    }
    std::string result(page,length);
    return result + ", checksum " + std::to_string(sum);
}

/* Adds an entry in the given disk region for the given virtual page number.
 * it creates a page using fill_page and stores it in disk.  page_num is
 * 0-indexed; so 0 is page base, 1 is page base + page_size, etc.
 */
void populate_disk_page(DiskRegion *disk, VPage base, size_t page_num, 
    const char *label) {
    char page[get_page_size()];
    fill_page(page, label, page_num);
    disk->store_data(base + page_num * get_page_size(), page);
}

void VMScript::run() {
    for (size_t i = 0; i < script.size(); i++) {
        if (script[i].e == READ) {
            VMInfo& info = regions[script[i].vm_id];

            // Read from the start of the given page
            cout << "Reading region #" << script[i].vm_id 
                 << ", page " << script[i].vpage_num << endl;
            char *p = info.vmregion->get_base() 
                      + script[i].vpage_num * get_page_size();
            read_byte(p);
            cout << VMRegion::log.c_str();
            VMRegion::log.clear();

            // If we previously stored data, print its info/signature
            if (info.stored_pages.find(script[i].vpage_num) 
                    != info.stored_pages.end()) {
                cout << "Region #" << script[i].vm_id << ", page " 
                     << script[i].vpage_num << " signature: " 
                     << page_signature(p).c_str() << endl;
                info.stored_pages.erase(script[i].vpage_num);
            }

            print_physmem_summary(physmem);
        } else if (script[i].e == WRITE) {
            VMInfo& info = regions[script[i].vm_id];

            // Write to the start of the given page
            cout << "Writing region #" << script[i].vm_id << ", page " 
                 << script[i].vpage_num << endl;
            char *p = info.vmregion->get_base() 
                      + script[i].vpage_num * get_page_size();
            fill_page(p, "write", script[i].vpage_num);
            info.stored_pages.insert(script[i].vpage_num);
            cout << VMRegion::log.c_str();
            VMRegion::log.clear();
            print_physmem_summary(physmem);
        } else if (script[i].e == STORE) {
            VMInfo& info = regions[script[i].vm_id];

            // Store made-up data for the page under the label __testNUM__
            string label = "__test";
            label += to_string(script[i].vpage_num);
            label += "__";
            cout << "Storing pre-populated data for region #" 
                 << script[i].vm_id << ", page #" 
                 << script[i].vpage_num << " with label " << label << endl;
            populate_disk_page(info.disk, info.vmregion->get_base(), 
                script[i].vpage_num, label.c_str());
            info.stored_pages.insert(script[i].vpage_num);
        } else if (script[i].e == SWEEP) {
            VMInfo& info = regions[script[i].vm_id];

            cout << "Calling clock_sweep() for region #" 
                 << script[i].vm_id << ", vpage #" 
                 << script[i].vpage_num << endl;

            char *p = info.vmregion->get_base() 
                      + script[i].vpage_num * get_page_size();
            info.vmregion->clock_sweep(p);
            std::cout << VMRegion::log.c_str();
            VMRegion::log.clear();
        } else if (script[i].e == REMOVE) {
            VMInfo& info = regions[script[i].vm_id];

            cout << "Calling clock_remove() for region #"
                 << script[i].vm_id << ", vpage #" 
                 << script[i].vpage_num << endl;

            char *p = info.vmregion->get_base() 
                      + script[i].vpage_num * get_page_size();
            info.vmregion->clock_remove(p);
            std::cout << VMRegion::log.c_str();
            VMRegion::log.clear();
            print_diskregion_summary(info.disk, script[i].vm_id);
        } else if (script[i].e == CHECK_SHOULD_REMOVE) {
            VMInfo& info = regions[script[i].vm_id];

            char *p = info.vmregion->get_base() 
                      + script[i].vpage_num * get_page_size();
            bool remove = info.vmregion->clock_should_remove(p);
            cout << "Region #" << script[i].vm_id 
                 << ", page #" << script[i].vpage_num 
                 << " candidate for replacement? " 
                 << (remove ? "yes" : "no") << endl;
        } else {
            cout << "ERROR" << endl;
        }
    }

    for (auto it = regions.begin(); it != regions.end(); ++it) {
        print_diskregion_summary((it->second).disk, it->first);
    }
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        VMScript script(argv[i]);
        cout << "Running script " << script.get_name() << "..." << endl;
        bool success = script.parse();
        if (success) {
            script.run();    
        } else {
            cerr << "ERROR: could not parse script file" << endl;
        }
    }
}
