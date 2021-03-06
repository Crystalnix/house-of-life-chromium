// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_CAPTURER_FAKE_ASCII_H_
#define REMOTING_HOST_CAPTURER_FAKE_ASCII_H_

#include "base/memory/scoped_ptr.h"
#include "remoting/host/capturer.h"
#include "remoting/host/capturer_helper.h"

namespace remoting {

// A CapturerFakeAscii always outputs an image of 64x48 ASCII characters.
// This image is artificially generated for testing purpose.
//
// CapturerFakeAscii is doubled buffered as required by Capturer. See
// remoting/host/capturer.h.
class CapturerFakeAscii : public Capturer {
 public:
  CapturerFakeAscii();
  ~CapturerFakeAscii();

  // Capturer interface.
  virtual void ScreenConfigurationChanged();
  virtual media::VideoFrame::Format pixel_format() const;
  virtual void ClearInvalidRects();
  virtual void InvalidateRects(const InvalidRects& inval_rects);
  virtual void InvalidateScreen(const gfx::Size& size);
  virtual void InvalidateFullScreen();
  virtual void CaptureInvalidRects(CaptureCompletedCallback* callback);
  virtual const gfx::Size& size_most_recent() const;

 private:
  // Generates an image in the front buffer.
  void GenerateImage();

  // The screen dimensions.
  int width_;
  int height_;
  int bytes_per_row_;

  CapturerHelper helper;

  // We have two buffers for the screen images as required by Capturer.
  static const int kNumBuffers = 2;
  scoped_array<uint8> buffers_[kNumBuffers];

  // The current buffer with valid data for reading.
  int current_buffer_;

  // Format of pixels returned in buffer.
  media::VideoFrame::Format pixel_format_;

  DISALLOW_COPY_AND_ASSIGN(CapturerFakeAscii);
};

}  // namespace remoting

#endif  // REMOTING_HOST_CAPTURER_FAKE_ASCII_H_
