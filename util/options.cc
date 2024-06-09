// Copyright (c) 2011 The MyDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "mydb/options.h"

#include "mydb/comparator.h"
#include "mydb/env.h"

namespace mydb {

Options::Options() : comparator(BytewiseComparator()), env(Env::Default()) {}

}  // namespace mydb
