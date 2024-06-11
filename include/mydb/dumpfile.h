

#ifndef STORAGE_MYDB_INCLUDE_DUMPFILE_H_
#define STORAGE_MYDB_INCLUDE_DUMPFILE_H_

#include <string>

#include "mydb/env.h"
#include "mydb/export.h"
#include "mydb/status.h"

namespace mydb {

// Dump the contents of the file named by fname in text format to
// *dst.  Makes a sequence of dst->Append() calls; each call is passed
// the newline-terminated text corresponding to a single item found
// in the file.
//
// Returns a non-OK result if fname does not name a mydb storage
// file, or if the file cannot be read.
MYDB_EXPORT Status DumpFile(Env* env, const std::string& fname,
                            WritableFile* dst);

} // namespace mydb

#endif // STORAGE_MYDB_INCLUDE_DUMPFILE_H_
