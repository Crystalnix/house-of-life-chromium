// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "chrome/browser/ui/cocoa/location_bar/ev_bubble_decoration.h"

#import "chrome/browser/ui/cocoa/cocoa_test_helper.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class EVBubbleDecorationTest : public CocoaTest {
 public:
  EVBubbleDecorationTest()
      : decoration_(NULL, [NSFont userFontOfSize:12]) {
  }

  EVBubbleDecoration decoration_;
};

// Test that the decoration gets smaller when there's not enough space
// to fit, within bounds.
TEST_F(EVBubbleDecorationTest, MiddleElide) {
  NSString* kLongString = @"A very long string with spaces";
  const CGFloat kWide = 1000.0;  // Wide enough to fit everything.
  const CGFloat kNarrow = 10.0;  // Too narrow for anything.
  const CGFloat kMinimumWidth = 100.0;  // Never should get this small.

  const NSSize kImageSize = NSMakeSize(20.0, 20.0);
  scoped_nsobject<NSImage> image([[NSImage alloc] initWithSize:kImageSize]);

  decoration_.SetImage(image);
  decoration_.SetFullLabel(kLongString);

  // Lots of space, decoration not omitted.
  EXPECT_NE(decoration_.GetWidthForSpace(kWide),
            LocationBarDecoration::kOmittedWidth);

  // If the available space is of the same magnitude as the required
  // space, the decoration doesn't eat it all up.
  const CGFloat long_width = decoration_.GetWidthForSpace(kWide);
  EXPECT_NE(decoration_.GetWidthForSpace(long_width + 20.0),
            LocationBarDecoration::kOmittedWidth);
  EXPECT_LT(decoration_.GetWidthForSpace(long_width + 20.0), long_width);

  // If there is very little space, the decoration is still relatively
  // big.
  EXPECT_NE(decoration_.GetWidthForSpace(kNarrow),
            LocationBarDecoration::kOmittedWidth);
  EXPECT_GT(decoration_.GetWidthForSpace(kNarrow), kMinimumWidth);
}

}  // namespace
