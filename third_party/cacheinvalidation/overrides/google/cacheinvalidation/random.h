// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GOOGLE_CACHEINVALIDATION_RANDOM_H_
#define GOOGLE_CACHEINVALIDATION_RANDOM_H_

#include "base/rand_util.h"

namespace invalidation {

class Random {
 public:
  // We don't actually use the seed.
  explicit Random(int64 seed) {}

  // Returns a pseudorandom value between(inclusive) and(exclusive).
  double RandDouble() {
    return base::RandDouble();
  }
};

}  // namespace invalidation

#endif  // GOOGLE_CACHEINVALIDATION_RANDOM_H_
