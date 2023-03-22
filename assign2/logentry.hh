
#pragma once

#include <stdexcept>
#include <variant>

#include "layout.hh"
#include "bufio.hh"

/* This file defines the on-disk data structures for the journaling
 * version of the V6 file system.
 */


/* LogPatch is a type of log entry that specifies a change to a piece of 
 * metadata on disk.  This is used for operations like updating an inode's 
 * stored block numbers, updating an inode's last modified time, updating an 
 * indirect block's block numbers, updating directory entries, etc.
 *
 * This entry contains specific bytes that must be written to a specific disk
 * block at a specific position in that block.  The byte range will not cross 
 * a sector boundary (though multiple LogPatch entries combined could modify 
 * different sectors).
 *
 *                  offset_in_block ────┐    ┌──────── bytes.size()
 *                                      │    │
 * 
 *                                     ◄─► ◄───►
 * ┌─────┬─────────────┬──────────────┬───┬─────┬─────────┬─────────────────┬──────┐
 * │     │             │              │    XXXXX          │                 │      │
 * │ ... │             │              │    XXXXX          │                 │ .... │
 * └─────┴─────────────┴──────────────┴───┴─────┴─────────┴─────────────────┴──────┘
 *  Disk blocks                       ▲      ▲
 *                                    │      │
 *                                  blockno  │
 *                                         bytes
 * Made with asciiflow.com
 */
struct LogPatch {
    uint16_t blockno;           // Block number to patch
    uint16_t offset_in_block;   // Offset within block of patch
    std::vector<uint8_t> bytes; // Bytes to place into the block

    // No need to worry about this
    static const char *type() { return "LogPatch"; }
    template<typename F> void archive(F &&f) {
        f("blockno", blockno);
        f("offset_in_block", offset_in_block);
        f("bytes", bytes);
    }
};

// LogBlockFree records that a previously allocated block is now free.
struct LogBlockFree {
    uint16_t blockno;           // Block number of freed block

    // No need to worry about this
    static const char *type() { return "LogBlockFree"; }
    template<typename F> void archive(F &&f) {
        f("blockno", blockno);
    }
};

/* LogBlockAlloc records that a previously free block was allocated
 * and marked allocated in the bitmap.  It specifies that the free
 * list should be updated, and also whether or not the block data
 * should be zeroed out (zero_on_replay).  zero_on_replay will be 
 * nonzero for new metadata blocks (like indirect blocks and 
 * directory contents, which are always zeroed out) and zero for data
 * blocks (actual file data), as zeroing them out may cause data loss.
 * (If the block needs to contain information, an additional LogPatch
 * entry would specify that).
 * 
 * The reason for this zeroing distinction is all future updates to 
 * metadata blocks are journaled to the log, so a freshly allocated 
 * metadata block should be zeroed out when replaying the log.  
 * However, a data block may reflect unlogged writes that were written 
 * back to disk between the allocation and a subsequent crash, 
 * so we do not want to zero out non-metadata blocks 
 * (thus zero_on_replay will be 0).
 */
struct LogBlockAlloc {
    uint16_t blockno;       // Block number that was allocated
    uint8_t zero_on_replay; // Metadata--should zero out block on replay

    // No need to worry about this
    static const char *type() { return "LogBlockAlloc"; }
    template<typename F> void archive(F &&f) {
        f("blockno", blockno);
        f("zero_on_replay", zero_on_replay);
    }
};


// You do not need to use anything in the remainder of the file below.

// The presence of a journal is
// indicated by a non-zero s_uselog field in the superblock.  If that
// byte is non-zero, then a second area of the disk image, following
// the V6 file system, includes the log area, consisting of a log
// header, a block bitmap, and a log.  The complete layout of the file
// system is thus:
//
//   +-----+-----+--------+------------------+---+------+---------+
//   |boot |super| inodes |      data        |log| free | journal |
//   |block|block|        |      blocks      |hdr| map  |  (log)  |
//   +-----+-----+--------+------------------+---+------+---------+
//   |     |     |        |                  |   |      |         |
//   0     1     2     2+s_isize         s_fsize*|      |    s_fsize +
//                                               |      |    l_logsize
//                                          1 + s_sfize |
//                                                      |
//                                             1 + s_fsize + l_logsize
//  * s_fsize == l_hdrblock


constexpr uint32_t LOG_MAGIC_NUM = 0x474c0636;
constexpr uint32_t LOG_CRC_SEED = 0x8ab27857;

// Each log entry has a log sequence number.
using lsn_t = uint32_t;

// Log header structure is the first block after the conventional V6
// file system.  It resides at block number s_fsize (where s_fsize is
// the superblock field expressing the number of blocks in the V6 file
// system).
struct loghdr {
    uint32_t l_magic;           // LOG_MAGIC_NUM
    uint32_t l_hdrblock;        // Block containing this log header

    // Total size of log in SECTOR_SIZE blocks.  File system plus log
    // area consume l_hdrblock+l_logsize sectors.
    uint16_t l_logsize;

    // Number of freemap blocks at start of log.
    uint16_t l_mapsize;

    // Byte offset of the first byte that should be read from the log
    // after a crash (measured from the start of the disk).
    uint32_t l_checkpoint;

    // First sequence number expected
    lsn_t l_sequence;

    char l_pad[SECTOR_SIZE-20];

    uint32_t mapstart() const { return l_hdrblock + 1; }
    uint32_t logstart() const { return mapstart() + l_mapsize; }
    uint32_t logend() const { return logstart() + l_logsize; }
    uint32_t logbytes() const {
        return SECTOR_SIZE * (l_logsize - l_mapsize - 1);
    }
};
static_assert(std::is_standard_layout_v<loghdr>,
              "on-disk data strcutures must have standard layout");
static_assert(sizeof(loghdr) == SECTOR_SIZE,
              "log_header must be exactly one sector");


//
// Types of log entry
//

// Mark the beginning of a transaction.  Every operation that changes
// the file system must be preceded by a LogBegin and followed by a
// matching LogCommit.  Otherwise, it means the log does not contain a
// full transaction, and a partial transaction must not be applied to
// the disk.  (A partial transaction can be discarded because the
// corresponding metadata will not have been written back to disk.)
struct LogBegin {
    static const char *type() { return "LogBegin"; }
    template<typename F> void archive(F &&f) {}
};

// Records that a transaction successfully committed.  Only valid if
// sequence is equal to the log sequence number of the previous
// LogBegin entry.
struct LogCommit {
    lsn_t sequence;          // Serial number of LogBegin

    static const char *type() { return "LogCommit"; }
    template<typename F> void archive(F &&f) {
        f("sequence", sequence);
    }
};

// The only operation that need not occur between a LogBegin and
// LogEnd, this entry indicates that the log wrapped around and the
// following log entry was written to the beginning of the log area.
struct LogRewind {
    static const char *type() { return "LogRewind"; }
    template<typename F> void archive(F &&f) {}
};

// A LogEntry is written out as a Header, followed by an entry (which
// is one of the above types), followed by a footer.
struct LogEntry {
    struct Header {
        lsn_t sequence;        // First copy of the sequence number
        uint8_t type;          // What type this is (entry_type index)
        template<typename F> void archive(F &&f) {
            f("sequence", sequence);
            f("type", type);
        }
    };
    using entry_type = std::variant<LogBegin,
                                    LogPatch,
                                    LogBlockAlloc,
                                    LogBlockFree,
                                    LogCommit,
                                    LogRewind>;
    struct Footer {
        uint32_t checksum;      // CRC-32 of header and object
        lsn_t sequence;         // Another copy of the sequence number
        template<typename F> void archive(F &&f) {
            f("checksum", checksum);
            f("sequence", sequence);
        }
    };

    lsn_t sequence_;            // LSN of this log entry
    entry_type entry_;          // Payload of this log entry

    LogEntry() : sequence_(0) {}
    template<typename T> LogEntry(lsn_t sn, T &&t)
        : sequence_(sn), entry_(std::forward<T>(t)) {}

    // Call an object f with whatever type is currently in entry_.  Example:
    //   struct F {
    //      void operator()(LogBegin &e) { ... }
    //      void operator()(LogPatch &e) { ... }
    //      ...
    //   };
    //   F f;
    //   le.visit(f);
    template<typename F> auto visit(F &&f) {
        return std::visit(std::forward<F>(f), entry_);
    }
    template<typename F> auto visit(F &&f) const {
        return std::visit(std::forward<F>(f), entry_);
    }

    // Use to retrieve a particular type of log entry or NULL.  Example:
    //   if (LogBegin *b = le.get<LogBegin>())
    //       do_something_with(b);
    template<typename T> T *get() { return std::get_if<T>(&entry_); }
    template<typename T> const T *get() const {
        return std::get_if<T>(&entry_);
    }

    void save(Writer &w) const;
    bool load(Reader &r);
    std::string show(const filsys *sb = nullptr) const;
    size_t nbytes() const;
};

// Exception thrown when deserializing corrupt log entires.  Should be
// caught, as it typically indicates the end of the log rather than a
// fatal error.
struct log_corrupt : std::runtime_error {
    using std::runtime_error::runtime_error;
};

uint32_t crc32(const void *_buf, size_t len, uint32_t seed);
std::string hexdump(const void *_p, size_t len);

// Extra information about patches
std::string what_patch(const filsys &sb, const LogPatch &e);
std::string what_inode_patch(const LogPatch &e);
std::string what_data_patch(const LogPatch &e);
