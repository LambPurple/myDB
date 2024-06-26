
//
// Log format information shared by reader and writer.
// See ../doc/log_format.md for more detail.

#ifndef STORAGE_MYDB_DB_LOG_FORMAT_H_
#define STORAGE_MYDB_DB_LOG_FORMAT_H_

namespace mydb {
namespace log {

enum RecordType {
    // Zero is reserved for preallocated files
    kZeroType = 0,

    kFullType = 1,

    // For fragments
    kFirstType = 2,
    kMiddleType = 3,
    kLastType = 4
};
static const int kMaxRecordType = kLastType;

static const int kBlockSize = 32768;

// Header is checksum (4 bytes), length (2 bytes), type (1 byte).
static const int kHeaderSize = 4 + 2 + 1;

} // namespace log
} // namespace mydb

#endif // STORAGE_MYDB_DB_LOG_FORMAT_H_
