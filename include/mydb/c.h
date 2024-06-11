/*

  C bindings for mydb.  May be useful as a stable ABI that can be
  used by programs that keep mydb in a shared library, or for
  a JNI api.

  Does not support:
  . getters for the option types
  . custom comparators that implement key shortening
  . custom iter, db, env, cache implementations using just the C bindings

  Some conventions:

  (1) We expose just opaque struct pointers and functions to clients.
  This allows us to change internal representations without having to
  recompile clients.

  (2) For simplicity, there is no equivalent to the Slice type.  Instead,
  the caller has to pass the pointer and length as separate
  arguments.

  (3) Errors are represented by a null-terminated c string.  NULL
  means no error.  All operations that can raise an error are passed
  a "char** errptr" as the last argument.  One of the following must
  be true on entry:
     *errptr == NULL
     *errptr points to a malloc()ed null-terminated error message
       (On Windows, *errptr must have been malloc()-ed by this library.)
  On success, a mydb routine leaves *errptr unchanged.
  On failure, mydb frees the old value of *errptr and
  set *errptr to a malloc()ed error message.

  (4) Bools have the type uint8_t (0 == false; rest == true)

  (5) All of the pointer arguments must be non-NULL.
*/

#ifndef STORAGE_MYDB_INCLUDE_C_H_
#define STORAGE_MYDB_INCLUDE_C_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "mydb/export.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types */

typedef struct mydb_t mydb_t;
typedef struct mydb_cache_t mydb_cache_t;
typedef struct mydb_comparator_t mydb_comparator_t;
typedef struct mydb_env_t mydb_env_t;
typedef struct mydb_filelock_t mydb_filelock_t;
typedef struct mydb_filterpolicy_t mydb_filterpolicy_t;
typedef struct mydb_iterator_t mydb_iterator_t;
typedef struct mydb_logger_t mydb_logger_t;
typedef struct mydb_options_t mydb_options_t;
typedef struct mydb_randomfile_t mydb_randomfile_t;
typedef struct mydb_readoptions_t mydb_readoptions_t;
typedef struct mydb_seqfile_t mydb_seqfile_t;
typedef struct mydb_snapshot_t mydb_snapshot_t;
typedef struct mydb_writablefile_t mydb_writablefile_t;
typedef struct mydb_writebatch_t mydb_writebatch_t;
typedef struct mydb_writeoptions_t mydb_writeoptions_t;

/* DB operations */

MYDB_EXPORT mydb_t* mydb_open(const mydb_options_t* options, const char* name,
                              char** errptr);

MYDB_EXPORT void mydb_close(mydb_t* db);

MYDB_EXPORT void mydb_put(mydb_t* db, const mydb_writeoptions_t* options,
                          const char* key, size_t keylen, const char* val,
                          size_t vallen, char** errptr);

MYDB_EXPORT void mydb_delete(mydb_t* db, const mydb_writeoptions_t* options,
                             const char* key, size_t keylen, char** errptr);

MYDB_EXPORT void mydb_write(mydb_t* db, const mydb_writeoptions_t* options,
                            mydb_writebatch_t* batch, char** errptr);

/* Returns NULL if not found.  A malloc()ed array otherwise.
   Stores the length of the array in *vallen. */
MYDB_EXPORT char* mydb_get(mydb_t* db, const mydb_readoptions_t* options,
                           const char* key, size_t keylen, size_t* vallen,
                           char** errptr);

MYDB_EXPORT mydb_iterator_t*
mydb_create_iterator(mydb_t* db, const mydb_readoptions_t* options);

MYDB_EXPORT const mydb_snapshot_t* mydb_create_snapshot(mydb_t* db);

MYDB_EXPORT void mydb_release_snapshot(mydb_t* db,
                                       const mydb_snapshot_t* snapshot);

/* Returns NULL if property name is unknown.
   Else returns a pointer to a malloc()-ed null-terminated value. */
MYDB_EXPORT char* mydb_property_value(mydb_t* db, const char* propname);

MYDB_EXPORT void mydb_approximate_sizes(mydb_t* db, int num_ranges,
                                        const char* const* range_start_key,
                                        const size_t* range_start_key_len,
                                        const char* const* range_limit_key,
                                        const size_t* range_limit_key_len,
                                        uint64_t* sizes);

MYDB_EXPORT void mydb_compact_range(mydb_t* db, const char* start_key,
                                    size_t start_key_len, const char* limit_key,
                                    size_t limit_key_len);

/* Management operations */

MYDB_EXPORT void mydb_destroy_db(const mydb_options_t* options,
                                 const char* name, char** errptr);

MYDB_EXPORT void mydb_repair_db(const mydb_options_t* options, const char* name,
                                char** errptr);

/* Iterator */

MYDB_EXPORT void mydb_iter_destroy(mydb_iterator_t*);
MYDB_EXPORT uint8_t mydb_iter_valid(const mydb_iterator_t*);
MYDB_EXPORT void mydb_iter_seek_to_first(mydb_iterator_t*);
MYDB_EXPORT void mydb_iter_seek_to_last(mydb_iterator_t*);
MYDB_EXPORT void mydb_iter_seek(mydb_iterator_t*, const char* k, size_t klen);
MYDB_EXPORT void mydb_iter_next(mydb_iterator_t*);
MYDB_EXPORT void mydb_iter_prev(mydb_iterator_t*);
MYDB_EXPORT const char* mydb_iter_key(const mydb_iterator_t*, size_t* klen);
MYDB_EXPORT const char* mydb_iter_value(const mydb_iterator_t*, size_t* vlen);
MYDB_EXPORT void mydb_iter_get_error(const mydb_iterator_t*, char** errptr);

/* Write batch */

MYDB_EXPORT mydb_writebatch_t* mydb_writebatch_create(void);
MYDB_EXPORT void mydb_writebatch_destroy(mydb_writebatch_t*);
MYDB_EXPORT void mydb_writebatch_clear(mydb_writebatch_t*);
MYDB_EXPORT void mydb_writebatch_put(mydb_writebatch_t*, const char* key,
                                     size_t klen, const char* val, size_t vlen);
MYDB_EXPORT void mydb_writebatch_delete(mydb_writebatch_t*, const char* key,
                                        size_t klen);
MYDB_EXPORT void mydb_writebatch_iterate(
    const mydb_writebatch_t*, void* state,
    void (*put)(void*, const char* k, size_t klen, const char* v, size_t vlen),
    void (*deleted)(void*, const char* k, size_t klen));
MYDB_EXPORT void mydb_writebatch_append(mydb_writebatch_t* destination,
                                        const mydb_writebatch_t* source);

/* Options */

MYDB_EXPORT mydb_options_t* mydb_options_create(void);
MYDB_EXPORT void mydb_options_destroy(mydb_options_t*);
MYDB_EXPORT void mydb_options_set_comparator(mydb_options_t*,
                                             mydb_comparator_t*);
MYDB_EXPORT void mydb_options_set_filter_policy(mydb_options_t*,
                                                mydb_filterpolicy_t*);
MYDB_EXPORT void mydb_options_set_create_if_missing(mydb_options_t*, uint8_t);
MYDB_EXPORT void mydb_options_set_error_if_exists(mydb_options_t*, uint8_t);
MYDB_EXPORT void mydb_options_set_paranoid_checks(mydb_options_t*, uint8_t);
MYDB_EXPORT void mydb_options_set_env(mydb_options_t*, mydb_env_t*);
MYDB_EXPORT void mydb_options_set_info_log(mydb_options_t*, mydb_logger_t*);
MYDB_EXPORT void mydb_options_set_write_buffer_size(mydb_options_t*, size_t);
MYDB_EXPORT void mydb_options_set_max_open_files(mydb_options_t*, int);
MYDB_EXPORT void mydb_options_set_cache(mydb_options_t*, mydb_cache_t*);
MYDB_EXPORT void mydb_options_set_block_size(mydb_options_t*, size_t);
MYDB_EXPORT void mydb_options_set_block_restart_interval(mydb_options_t*, int);
MYDB_EXPORT void mydb_options_set_max_file_size(mydb_options_t*, size_t);

enum { mydb_no_compression = 0, mydb_snappy_compression = 1 };
MYDB_EXPORT void mydb_options_set_compression(mydb_options_t*, int);

/* Comparator */

MYDB_EXPORT mydb_comparator_t*
mydb_comparator_create(void* state, void (*destructor)(void*),
                       int (*compare)(void*, const char* a, size_t alen,
                                      const char* b, size_t blen),
                       const char* (*name)(void*));
MYDB_EXPORT void mydb_comparator_destroy(mydb_comparator_t*);

/* Filter policy */

MYDB_EXPORT mydb_filterpolicy_t* mydb_filterpolicy_create(
    void* state, void (*destructor)(void*),
    char* (*create_filter)(void*, const char* const* key_array,
                           const size_t* key_length_array, int num_keys,
                           size_t* filter_length),
    uint8_t (*key_may_match)(void*, const char* key, size_t length,
                             const char* filter, size_t filter_length),
    const char* (*name)(void*));
MYDB_EXPORT void mydb_filterpolicy_destroy(mydb_filterpolicy_t*);

MYDB_EXPORT mydb_filterpolicy_t*
mydb_filterpolicy_create_bloom(int bits_per_key);

/* Read options */

MYDB_EXPORT mydb_readoptions_t* mydb_readoptions_create(void);
MYDB_EXPORT void mydb_readoptions_destroy(mydb_readoptions_t*);
MYDB_EXPORT void mydb_readoptions_set_verify_checksums(mydb_readoptions_t*,
                                                       uint8_t);
MYDB_EXPORT void mydb_readoptions_set_fill_cache(mydb_readoptions_t*, uint8_t);
MYDB_EXPORT void mydb_readoptions_set_snapshot(mydb_readoptions_t*,
                                               const mydb_snapshot_t*);

/* Write options */

MYDB_EXPORT mydb_writeoptions_t* mydb_writeoptions_create(void);
MYDB_EXPORT void mydb_writeoptions_destroy(mydb_writeoptions_t*);
MYDB_EXPORT void mydb_writeoptions_set_sync(mydb_writeoptions_t*, uint8_t);

/* Cache */

MYDB_EXPORT mydb_cache_t* mydb_cache_create_lru(size_t capacity);
MYDB_EXPORT void mydb_cache_destroy(mydb_cache_t* cache);

/* Env */

MYDB_EXPORT mydb_env_t* mydb_create_default_env(void);
MYDB_EXPORT void mydb_env_destroy(mydb_env_t*);

/* If not NULL, the returned buffer must be released using mydb_free(). */
MYDB_EXPORT char* mydb_env_get_test_directory(mydb_env_t*);

/* Utility */

/* Calls free(ptr).
   REQUIRES: ptr was malloc()-ed and returned by one of the routines
   in this file.  Note that in certain cases (typically on Windows), you
   may need to call this routine instead of free(ptr) to dispose of
   malloc()-ed memory returned by this library. */
MYDB_EXPORT void mydb_free(void* ptr);

/* Return the major version number for this release. */
MYDB_EXPORT int mydb_major_version(void);

/* Return the minor version number for this release. */
MYDB_EXPORT int mydb_minor_version(void);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* STORAGE_MYDB_INCLUDE_C_H_ */
