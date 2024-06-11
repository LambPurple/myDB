

#include "mydb/options.h"

#include "mydb/comparator.h"
#include "mydb/env.h"

namespace mydb {

Options::Options() : comparator(BytewiseComparator()), env(Env::Default()) {}

}  // namespace mydb
