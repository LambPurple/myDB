
//
// Simple hash function used for internal data structures

#ifndef STORAGE_MYDB_UTIL_HASH_H_
#define STORAGE_MYDB_UTIL_HASH_H_

#include <cstddef>
#include <cstdint>

namespace mydb {

uint32_t Hash(const char* data, size_t n, uint32_t seed);

}  // namespace mydb

#endif  // STORAGE_MYDB_UTIL_HASH_H_
