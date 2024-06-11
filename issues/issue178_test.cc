// Copyright (c) 2013 The MyDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

// Test for issue 178: a manual compaction causes deleted data to reappear.
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "mydb/db.h"
#include "mydb/write_batch.h"

#include "util/testutil.h"

#include "gtest/gtest.h"

namespace {

const int kNumKeys = 1100000;

std::string Key1(int i) {
    char buf[100];
    std::snprintf(buf, sizeof(buf), "my_key_%d", i);
    return buf;
}

std::string Key2(int i) { return Key1(i) + "_xxx"; }

TEST(Issue178, Test) {
    // Get rid of any state from an old run.
    std::string dbpath = testing::TempDir() + "mydb_cbug_test";
    DestroyDB(dbpath, mydb::Options());

    // Open database.  Disable compression since it affects the creation
    // of layers and the code below is trying to test against a very
    // specific scenario.
    mydb::DB* db;
    mydb::Options db_options;
    db_options.create_if_missing = true;
    db_options.compression = mydb::kNoCompression;
    ASSERT_MYDB_OK(mydb::DB::Open(db_options, dbpath, &db));

    // create first key range
    mydb::WriteBatch batch;
    for (size_t i = 0; i < kNumKeys; i++) {
        batch.Put(Key1(i), "value for range 1 key");
    }
    ASSERT_MYDB_OK(db->Write(mydb::WriteOptions(), &batch));

    // create second key range
    batch.Clear();
    for (size_t i = 0; i < kNumKeys; i++) {
        batch.Put(Key2(i), "value for range 2 key");
    }
    ASSERT_MYDB_OK(db->Write(mydb::WriteOptions(), &batch));

    // delete second key range
    batch.Clear();
    for (size_t i = 0; i < kNumKeys; i++) {
        batch.Delete(Key2(i));
    }
    ASSERT_MYDB_OK(db->Write(mydb::WriteOptions(), &batch));

    // compact database
    std::string start_key = Key1(0);
    std::string end_key = Key1(kNumKeys - 1);
    mydb::Slice least(start_key.data(), start_key.size());
    mydb::Slice greatest(end_key.data(), end_key.size());

    // commenting out the line below causes the example to work correctly
    db->CompactRange(&least, &greatest);

    // count the keys
    mydb::Iterator* iter = db->NewIterator(mydb::ReadOptions());
    size_t num_keys = 0;
    for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        num_keys++;
    }
    delete iter;
    ASSERT_EQ(kNumKeys, num_keys) << "Bad number of keys";

    // close database
    delete db;
    DestroyDB(dbpath, mydb::Options());
}

} // anonymous namespace
