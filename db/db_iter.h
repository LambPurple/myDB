

#ifndef STORAGE_MYDB_DB_DB_ITER_H_
#define STORAGE_MYDB_DB_DB_ITER_H_

#include "db/dbformat.h"
#include <cstdint>

#include "mydb/db.h"

namespace mydb {

class DBImpl;

// Return a new iterator that converts internal keys (yielded by
// "*internal_iter") that were live at the specified "sequence" number
// into appropriate user keys.
Iterator* NewDBIterator(DBImpl* db, const Comparator* user_key_comparator,
                        Iterator* internal_iter, SequenceNumber sequence,
                        uint32_t seed);

} // namespace mydb

#endif // STORAGE_MYDB_DB_DB_ITER_H_
