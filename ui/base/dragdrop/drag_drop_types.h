// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_DRAGDROP_DRAG_DROP_TYPES_H_
#define UI_BASE_DRAGDROP_DRAG_DROP_TYPES_H_
#pragma once

#include "build/build_config.h"

#include "base/basictypes.h"

namespace ui {

class DragDropTypes {
 public:
  enum DragOperation {
    DRAG_NONE = 0,
    DRAG_MOVE = 1 << 0,
    DRAG_COPY = 1 << 1,
    DRAG_LINK = 1 << 2
  };

#if defined(OS_WIN)
  static uint32 DragOperationToDropEffect(int drag_operation);
  static int DropEffectToDragOperation(uint32 effect);
#elif !defined(OS_MACOSX)
  static int DragOperationToGdkDragAction(int drag_operation);
  static int GdkDragActionToDragOperation(int gdk_drag_action);
#endif
};

}  // namespace ui

#endif  // UI_BASE_DRAGDROP_DRAG_DROP_TYPES_H_
