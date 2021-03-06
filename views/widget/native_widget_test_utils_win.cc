// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/widget/native_widget_test_utils.h"

#include "views/view.h"
#include "views/widget/widget_win.h"

namespace views {
namespace internal {

NativeWidget* CreateNativeWidget() {
  return CreateNativeWidgetWithContents(new View);
}

NativeWidget* CreateNativeWidgetWithContents(View* contents_view) {
  Widget* widget = Widget::CreateWidget();
  Widget::InitParams params(Widget::InitParams::TYPE_WINDOW);
  params.delete_on_destroy = false;
  params.bounds = gfx::Rect(10, 10, 200, 200);
  widget->Init(params);
  return widget->native_widget();
}

NativeWidget* CreateNativeWidgetWithParent(NativeWidget* parent) {
  Widget* widget = Widget::CreateWidget();
  Widget::InitParams params(Widget::InitParams::TYPE_CONTROL);
  params.delete_on_destroy = false;
  params.child = false; // Implicitly set to true by ctor with TYPE_CONTROL.
  params.parent = parent ? parent->GetWidget()->GetNativeView() : NULL;
  params.bounds = gfx::Rect(10, 10, 200, 200);
  widget->Init(params);
  return widget->native_widget();
}

}  // namespace internal
}  // namespace ui
