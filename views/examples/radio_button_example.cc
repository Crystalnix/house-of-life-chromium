// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "views/examples/radio_button_example.h"

#include "base/stringprintf.h"
#include "views/controls/button/text_button.h"
#include "views/layout/grid_layout.h"
#include "views/view.h"

namespace examples {

RadioButtonExample::RadioButtonExample(ExamplesMain* main)
    : ExampleBase(main), count_(0) {
}

RadioButtonExample::~RadioButtonExample() {
}

std::wstring RadioButtonExample::GetExampleTitle() {
  return L"Radio Button";
}

void RadioButtonExample::CreateExampleView(views::View* container) {
  select_ = new views::TextButton(this, L"Select");
  status_ = new views::TextButton(this, L"Show Status");

  int group = 1;
  for (size_t i = 0; i < arraysize(native_radio_buttons_); ++i) {
    native_radio_buttons_[i] = new views::NativeRadioButton(
        base::StringPrintf( L"Radio %d in group %d", i + 1, group), group);
    native_radio_buttons_[i]->set_listener(this);
  }

  ++group;
  for (size_t i = 0; i < arraysize(radio_buttons_); ++i) {
    radio_buttons_[i] = new views::RadioButton(
        base::StringPrintf( L"Radio %d in group %d", i + 1, group), group);
    radio_buttons_[i]->set_listener(this);
  }

  views::GridLayout* layout = new views::GridLayout(container);
  container->SetLayoutManager(layout);

  views::ColumnSet* column_set = layout->AddColumnSet(0);
  column_set->AddColumn(views::GridLayout::FILL, views::GridLayout::FILL,
                        1.0f, views::GridLayout::USE_PREF, 0, 0);
  for (size_t i = 0; i < arraysize(native_radio_buttons_); ++i) {
    layout->StartRow(0, 0);
    layout->AddView(native_radio_buttons_[i]);
  }
  for (size_t i = 0; i < arraysize(radio_buttons_); ++i) {
    layout->StartRow(0, 0);
    layout->AddView(radio_buttons_[i]);
  }
  layout->StartRow(0, 0);
  layout->AddView(select_);
  layout->StartRow(0, 0);
  layout->AddView(status_);
}

void RadioButtonExample::ButtonPressed(views::Button* sender,
                                       const views::Event& event) {
  if (sender == select_) {
    native_radio_buttons_[0]->SetChecked(true);
    radio_buttons_[2]->SetChecked(true);
  } else if (sender == status_) {
    // Show the state of radio buttons.
    PrintStatus(L"Group1: 1:%ls, 2:%ls, 3:%ls   Group2: 1:%ls, 2:%ls, 3:%ls",
        IntToOnOff(native_radio_buttons_[0]->checked()),
        IntToOnOff(native_radio_buttons_[1]->checked()),
        IntToOnOff(native_radio_buttons_[2]->checked()),
        IntToOnOff(radio_buttons_[0]->checked()),
        IntToOnOff(radio_buttons_[1]->checked()),
        IntToOnOff(radio_buttons_[2]->checked()));
  } else {
    PrintStatus(L"Pressed! count:%d", ++count_);
  }
}

}  // namespace examples
