// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_MESSAGE_BOX_WIN_H_
#define UI_BASE_MESSAGE_BOX_WIN_H_
#pragma once

#include <windows.h>

#include "base/string16.h"

namespace ui {

// A wrapper around Windows' MessageBox function. Using a Chrome specific
// MessageBox function allows us to control certain RTL locale flags so that
// callers don't have to worry about adding these flags when running in a
// right-to-left locale.
int MessageBox(HWND hwnd,
               const string16& text,
               const string16& caption,
               UINT flags);

}  // namespace ui

#endif  // UI_BASE_MESSAGE_BOX_WIN_H_
