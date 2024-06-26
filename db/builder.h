

#ifndef STORAGE_MYDB_DB_BUILDER_H_
#define STORAGE_MYDB_DB_BUILDER_H_

#include "mydb/status.h"

namespace mydb {

struct Options;
struct FileMetaData;

class Env;
class Iterator;
class TableCache;
class VersionEdit;

// Build a Table file from the contents of *iter.  The generated file
// will be named according to meta->number.  On success, the rest of
// *meta will be filled with metadata about the generated table.
// If no data is present in *iter, meta->file_size will be set to
// zero, and no Table file will be produced.
Status BuildTable(const std::string& dbname, Env* env, const Options& options,
                  TableCache* table_cache, Iterator* iter, FileMetaData* meta);

} // namespace mydb

#endif // STORAGE_MYDB_DB_BUILDER_H_
