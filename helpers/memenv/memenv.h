

#ifndef STORAGE_MYDB_HELPERS_MEMENV_MEMENV_H_
#define STORAGE_MYDB_HELPERS_MEMENV_MEMENV_H_

#include "mydb/export.h"

namespace mydb {

class Env;

// Returns a new environment that stores its data in memory and delegates
// all non-file-storage tasks to base_env. The caller must delete the result
// when it is no longer needed.
// *base_env must remain live while the result is in use.
MYDB_EXPORT Env* NewMemEnv(Env* base_env);

} // namespace mydb

#endif // STORAGE_MYDB_HELPERS_MEMENV_MEMENV_H_
