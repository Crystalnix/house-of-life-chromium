// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_TEST_SUITE_H_
#define UI_GFX_TEST_SUITE_H_
#pragma once

#include <string>

#include "base/test/test_suite.h"
#include "build/build_config.h"

class GfxTestSuite : public base::TestSuite {
 public:
  GfxTestSuite(int argc, char** argv);

 protected:
  // Overridden from base::TestSuite:
  virtual void Initialize();
  virtual void Shutdown();
};

#endif  // UI_GFX_TEST_SUITE_H_
