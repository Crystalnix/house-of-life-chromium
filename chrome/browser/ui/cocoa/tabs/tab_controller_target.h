// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_COCOA_TABS_TAB_CONTROLLER_TARGET_H_
#define CHROME_BROWSER_UI_COCOA_TABS_TAB_CONTROLLER_TARGET_H_
#pragma once

#include "chrome/browser/tabs/tab_strip_model.h"
#include "chrome/browser/ui/tabs/tab_menu_model.h"

@class TabController;

// A protocol to be implemented by a TabController's target.
@protocol TabControllerTarget
- (void)selectTab:(id)sender;
- (void)closeTab:(id)sender;

// Dispatch context menu commands for the given tab controller.
- (void)commandDispatch:(TabStripModel::ContextMenuCommand)command
          forController:(TabController*)controller;
// Returns YES if the specificed command should be enabled for the given
// controller.
- (BOOL)isCommandEnabled:(TabStripModel::ContextMenuCommand)command
           forController:(TabController*)controller;

// Returns a context menu model for a given controller. Caller owns the result.
- (ui::SimpleMenuModel*)contextMenuModelForController:(TabController*)controller
    menuDelegate:(ui::SimpleMenuModel::Delegate*)delegate;

@end

#endif  // CHROME_BROWSER_UI_COCOA_TABS_TAB_CONTROLLER_TARGET_H_
