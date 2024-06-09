/* Copyright (c) 2011 The MyDB Authors. All rights reserved.
   Use of this source code is governed by a BSD-style license that can be
   found in the LICENSE file. See the AUTHORS file for names of contributors. */

#include "mydb/c.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* phase = "";

static void StartPhase(const char* name) {
  fprintf(stderr, "=== Test %s\n", name);
  phase = name;
}

#define CheckNoError(err)                                               \
  if ((err) != NULL) {                                                  \
    fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, phase, (err)); \
    abort();                                                            \
  }

#define CheckCondition(cond)                                            \
  if (!(cond)) {                                                        \
    fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, phase, #cond); \
    abort();                                                            \
  }

static void CheckEqual(const char* expected, const char* v, size_t n) {
  if (expected == NULL && v == NULL) {
    // ok
  } else if (expected != NULL && v != NULL && n == strlen(expected) &&
             memcmp(expected, v, n) == 0) {
    // ok
    return;
  } else {
    fprintf(stderr, "%s: expected '%s', got '%s'\n",
            phase,
            (expected ? expected : "(null)"),
            (v ? v : "(null"));
    abort();
  }
}

static void Free(char** ptr) {
  if (*ptr) {
    free(*ptr);
    *ptr = NULL;
  }
}

static void CheckGet(
    mydb_t* db,
    const mydb_readoptions_t* options,
    const char* key,
    const char* expected) {
  char* err = NULL;
  size_t val_len;
  char* val;
  val = mydb_get(db, options, key, strlen(key), &val_len, &err);
  CheckNoError(err);
  CheckEqual(expected, val, val_len);
  Free(&val);
}

static void CheckIter(mydb_iterator_t* iter,
                      const char* key, const char* val) {
  size_t len;
  const char* str;
  str = mydb_iter_key(iter, &len);
  CheckEqual(key, str, len);
  str = mydb_iter_value(iter, &len);
  CheckEqual(val, str, len);
}

// Callback from mydb_writebatch_iterate()
static void CheckPut(void* ptr,
                     const char* k, size_t klen,
                     const char* v, size_t vlen) {
  int* state = (int*) ptr;
  CheckCondition(*state < 2);
  switch (*state) {
    case 0:
      CheckEqual("bar", k, klen);
      CheckEqual("b", v, vlen);
      break;
    case 1:
      CheckEqual("box", k, klen);
      CheckEqual("c", v, vlen);
      break;
  }
  (*state)++;
}

// Callback from mydb_writebatch_iterate()
static void CheckDel(void* ptr, const char* k, size_t klen) {
  int* state = (int*) ptr;
  CheckCondition(*state == 2);
  CheckEqual("bar", k, klen);
  (*state)++;
}

static void CmpDestroy(void* arg) { }

static int CmpCompare(void* arg, const char* a, size_t alen,
                      const char* b, size_t blen) {
  int n = (alen < blen) ? alen : blen;
  int r = memcmp(a, b, n);
  if (r == 0) {
    if (alen < blen) r = -1;
    else if (alen > blen) r = +1;
  }
  return r;
}

static const char* CmpName(void* arg) {
  return "foo";
}

// Custom filter policy
static uint8_t fake_filter_result = 1;
static void FilterDestroy(void* arg) { }
static const char* FilterName(void* arg) {
  return "TestFilter";
}
static char* FilterCreate(
    void* arg,
    const char* const* key_array, const size_t* key_length_array,
    int num_keys,
    size_t* filter_length) {
  *filter_length = 4;
  char* result = malloc(4);
  memcpy(result, "fake", 4);
  return result;
}
uint8_t FilterKeyMatch(void* arg, const char* key, size_t length,
                       const char* filter, size_t filter_length) {
  CheckCondition(filter_length == 4);
  CheckCondition(memcmp(filter, "fake", 4) == 0);
  return fake_filter_result;
}

int main(int argc, char** argv) {
  mydb_t* db;
  mydb_comparator_t* cmp;
  mydb_cache_t* cache;
  mydb_env_t* env;
  mydb_options_t* options;
  mydb_readoptions_t* roptions;
  mydb_writeoptions_t* woptions;
  char* dbname;
  char* err = NULL;
  int run = -1;

  CheckCondition(mydb_major_version() >= 1);
  CheckCondition(mydb_minor_version() >= 1);

  StartPhase("create_objects");
  cmp = mydb_comparator_create(NULL, CmpDestroy, CmpCompare, CmpName);
  env = mydb_create_default_env();
  cache = mydb_cache_create_lru(100000);
  dbname = mydb_env_get_test_directory(env);
  CheckCondition(dbname != NULL);

  options = mydb_options_create();
  mydb_options_set_comparator(options, cmp);
  mydb_options_set_error_if_exists(options, 1);
  mydb_options_set_cache(options, cache);
  mydb_options_set_env(options, env);
  mydb_options_set_info_log(options, NULL);
  mydb_options_set_write_buffer_size(options, 100000);
  mydb_options_set_paranoid_checks(options, 1);
  mydb_options_set_max_open_files(options, 10);
  mydb_options_set_block_size(options, 1024);
  mydb_options_set_block_restart_interval(options, 8);
  mydb_options_set_max_file_size(options, 3 << 20);
  mydb_options_set_compression(options, mydb_no_compression);

  roptions = mydb_readoptions_create();
  mydb_readoptions_set_verify_checksums(roptions, 1);
  mydb_readoptions_set_fill_cache(roptions, 0);

  woptions = mydb_writeoptions_create();
  mydb_writeoptions_set_sync(woptions, 1);

  StartPhase("destroy");
  mydb_destroy_db(options, dbname, &err);
  Free(&err);

  StartPhase("open_error");
  db = mydb_open(options, dbname, &err);
  CheckCondition(err != NULL);
  Free(&err);

  StartPhase("mydb_free");
  db = mydb_open(options, dbname, &err);
  CheckCondition(err != NULL);
  mydb_free(err);
  err = NULL;

  StartPhase("open");
  mydb_options_set_create_if_missing(options, 1);
  db = mydb_open(options, dbname, &err);
  CheckNoError(err);
  CheckGet(db, roptions, "foo", NULL);

  StartPhase("put");
  mydb_put(db, woptions, "foo", 3, "hello", 5, &err);
  CheckNoError(err);
  CheckGet(db, roptions, "foo", "hello");

  StartPhase("compactall");
  mydb_compact_range(db, NULL, 0, NULL, 0);
  CheckGet(db, roptions, "foo", "hello");

  StartPhase("compactrange");
  mydb_compact_range(db, "a", 1, "z", 1);
  CheckGet(db, roptions, "foo", "hello");

  StartPhase("writebatch");
  {
    mydb_writebatch_t* wb = mydb_writebatch_create();
    mydb_writebatch_put(wb, "foo", 3, "a", 1);
    mydb_writebatch_clear(wb);
    mydb_writebatch_put(wb, "bar", 3, "b", 1);
    mydb_writebatch_put(wb, "box", 3, "c", 1);

    mydb_writebatch_t* wb2 = mydb_writebatch_create();
    mydb_writebatch_delete(wb2, "bar", 3);
    mydb_writebatch_append(wb, wb2);
    mydb_writebatch_destroy(wb2);

    mydb_write(db, woptions, wb, &err);
    CheckNoError(err);
    CheckGet(db, roptions, "foo", "hello");
    CheckGet(db, roptions, "bar", NULL);
    CheckGet(db, roptions, "box", "c");

    int pos = 0;
    mydb_writebatch_iterate(wb, &pos, CheckPut, CheckDel);
    CheckCondition(pos == 3);
    mydb_writebatch_destroy(wb);
  }

  StartPhase("iter");
  {
    mydb_iterator_t* iter = mydb_create_iterator(db, roptions);
    CheckCondition(!mydb_iter_valid(iter));
    mydb_iter_seek_to_first(iter);
    CheckCondition(mydb_iter_valid(iter));
    CheckIter(iter, "box", "c");
    mydb_iter_next(iter);
    CheckIter(iter, "foo", "hello");
    mydb_iter_prev(iter);
    CheckIter(iter, "box", "c");
    mydb_iter_prev(iter);
    CheckCondition(!mydb_iter_valid(iter));
    mydb_iter_seek_to_last(iter);
    CheckIter(iter, "foo", "hello");
    mydb_iter_seek(iter, "b", 1);
    CheckIter(iter, "box", "c");
    mydb_iter_get_error(iter, &err);
    CheckNoError(err);
    mydb_iter_destroy(iter);
  }

  StartPhase("approximate_sizes");
  {
    int i;
    int n = 20000;
    char keybuf[100];
    char valbuf[100];
    uint64_t sizes[2];
    const char* start[2] = { "a", "k00000000000000010000" };
    size_t start_len[2] = { 1, 21 };
    const char* limit[2] = { "k00000000000000010000", "z" };
    size_t limit_len[2] = { 21, 1 };
    mydb_writeoptions_set_sync(woptions, 0);
    for (i = 0; i < n; i++) {
      snprintf(keybuf, sizeof(keybuf), "k%020d", i);
      snprintf(valbuf, sizeof(valbuf), "v%020d", i);
      mydb_put(db, woptions, keybuf, strlen(keybuf), valbuf, strlen(valbuf),
                  &err);
      CheckNoError(err);
    }
    mydb_approximate_sizes(db, 2, start, start_len, limit, limit_len, sizes);
    CheckCondition(sizes[0] > 0);
    CheckCondition(sizes[1] > 0);
  }

  StartPhase("property");
  {
    char* prop = mydb_property_value(db, "nosuchprop");
    CheckCondition(prop == NULL);
    prop = mydb_property_value(db, "mydb.stats");
    CheckCondition(prop != NULL);
    Free(&prop);
  }

  StartPhase("snapshot");
  {
    const mydb_snapshot_t* snap;
    snap = mydb_create_snapshot(db);
    mydb_delete(db, woptions, "foo", 3, &err);
    CheckNoError(err);
    mydb_readoptions_set_snapshot(roptions, snap);
    CheckGet(db, roptions, "foo", "hello");
    mydb_readoptions_set_snapshot(roptions, NULL);
    CheckGet(db, roptions, "foo", NULL);
    mydb_release_snapshot(db, snap);
  }

  StartPhase("repair");
  {
    mydb_close(db);
    mydb_options_set_create_if_missing(options, 0);
    mydb_options_set_error_if_exists(options, 0);
    mydb_repair_db(options, dbname, &err);
    CheckNoError(err);
    db = mydb_open(options, dbname, &err);
    CheckNoError(err);
    CheckGet(db, roptions, "foo", NULL);
    CheckGet(db, roptions, "bar", NULL);
    CheckGet(db, roptions, "box", "c");
    mydb_options_set_create_if_missing(options, 1);
    mydb_options_set_error_if_exists(options, 1);
  }

  StartPhase("filter");
  for (run = 0; run < 2; run++) {
    // First run uses custom filter, second run uses bloom filter
    CheckNoError(err);
    mydb_filterpolicy_t* policy;
    if (run == 0) {
      policy = mydb_filterpolicy_create(
          NULL, FilterDestroy, FilterCreate, FilterKeyMatch, FilterName);
    } else {
      policy = mydb_filterpolicy_create_bloom(10);
    }

    // Create new database
    mydb_close(db);
    mydb_destroy_db(options, dbname, &err);
    mydb_options_set_filter_policy(options, policy);
    db = mydb_open(options, dbname, &err);
    CheckNoError(err);
    mydb_put(db, woptions, "foo", 3, "foovalue", 8, &err);
    CheckNoError(err);
    mydb_put(db, woptions, "bar", 3, "barvalue", 8, &err);
    CheckNoError(err);
    mydb_compact_range(db, NULL, 0, NULL, 0);

    fake_filter_result = 1;
    CheckGet(db, roptions, "foo", "foovalue");
    CheckGet(db, roptions, "bar", "barvalue");
    if (phase == 0) {
      // Must not find value when custom filter returns false
      fake_filter_result = 0;
      CheckGet(db, roptions, "foo", NULL);
      CheckGet(db, roptions, "bar", NULL);
      fake_filter_result = 1;

      CheckGet(db, roptions, "foo", "foovalue");
      CheckGet(db, roptions, "bar", "barvalue");
    }
    mydb_options_set_filter_policy(options, NULL);
    mydb_filterpolicy_destroy(policy);
  }

  StartPhase("cleanup");
  mydb_close(db);
  mydb_options_destroy(options);
  mydb_readoptions_destroy(roptions);
  mydb_writeoptions_destroy(woptions);
  mydb_free(dbname);
  mydb_cache_destroy(cache);
  mydb_comparator_destroy(cmp);
  mydb_env_destroy(env);

  fprintf(stderr, "PASS\n");
  return 0;
}
