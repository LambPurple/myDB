

#ifndef STORAGE_MYDB_TABLE_TWO_LEVEL_ITERATOR_H_
#define STORAGE_MYDB_TABLE_TWO_LEVEL_ITERATOR_H_

#include "mydb/iterator.h"

namespace mydb {

struct ReadOptions;

// Return a new two level iterator.  A two-level iterator contains an
// index iterator whose values point to a sequence of blocks where
// each block is itself a sequence of key,value pairs.  The returned
// two-level iterator yields the concatenation of all key/value pairs
// in the sequence of blocks.  Takes ownership of "index_iter" and
// will delete it when no longer needed.
//
// Uses a supplied function to convert an index_iter value into
// an iterator over the contents of the corresponding block.
Iterator* NewTwoLevelIterator(
    Iterator* index_iter,
    Iterator* (*block_function)(void* arg, const ReadOptions& options,
                                const Slice& index_value),
    void* arg, const ReadOptions& options);

} // namespace mydb

#endif // STORAGE_MYDB_TABLE_TWO_LEVEL_ITERATOR_H_
