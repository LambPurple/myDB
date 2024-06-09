// Copyright (c) 2011 The MyDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
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
