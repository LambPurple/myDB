

#ifndef STORAGE_MYDB_TABLE_BLOCK_BUILDER_H_
#define STORAGE_MYDB_TABLE_BLOCK_BUILDER_H_

#include <cstdint>
#include <vector>

#include "mydb/slice.h"

namespace mydb {

struct Options;

class BlockBuilder {
  public:
    explicit BlockBuilder(const Options* options);

    BlockBuilder(const BlockBuilder&) = delete;
    BlockBuilder& operator=(const BlockBuilder&) = delete;

    // Reset the contents as if the BlockBuilder was just constructed.
    void Reset();

    // REQUIRES: Finish() has not been called since the last call to Reset().
    // REQUIRES: key is larger than any previously added key
    void Add(const Slice& key, const Slice& value);

    // Finish building the block and return a slice that refers to the
    // block contents.  The returned slice will remain valid for the
    // lifetime of this builder or until Reset() is called.
    Slice Finish();

    // Returns an estimate of the current (uncompressed) size of the block
    // we are building.
    size_t CurrentSizeEstimate() const;

    // Return true iff no entries have been added since the last Reset()
    bool empty() const { return buffer_.empty(); }

  private:
    const Options* options_;
    std::string buffer_;             // Destination buffer
    std::vector<uint32_t> restarts_; // Restart points
    int counter_;                    // Number of entries emitted since restart
    bool finished_;                  // Has Finish() been called?
    std::string last_key_;
};

} // namespace mydb

#endif // STORAGE_MYDB_TABLE_BLOCK_BUILDER_H_
