// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_BOOKMARKS_BOOKMARKS_TAB_HELPER_H_
#define CHROME_BROWSER_UI_BOOKMARKS_BOOKMARKS_TAB_HELPER_H_
#pragma once

#include "content/browser/tab_contents/tab_contents_observer.h"
#include "content/common/notification_observer.h"
#include "content/common/notification_registrar.h"

class BookmarksTabHelperDelegate;
class TabContentsWrapper;

// Per-tab class to manage bookmarks.
class BookmarksTabHelper : public NotificationObserver,
                           public TabContentsObserver {
 public:
  explicit BookmarksTabHelper(TabContentsWrapper* tab_contents);
  virtual ~BookmarksTabHelper();

  bool is_starred() const { return is_starred_; }

  BookmarksTabHelperDelegate* delegate() const { return delegate_; }
  void set_delegate(BookmarksTabHelperDelegate* d) { delegate_ = d; }

  // TabContentsObserver overrides:
  virtual void DidNavigateMainFramePostCommit(
      const NavigationController::LoadCommittedDetails& details,
      const ViewHostMsg_FrameNavigate_Params& params) OVERRIDE;

  // NotificationObserver overrides:
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details) OVERRIDE;

 private:
  // Updates the starred state from the bookmark bar model. If the state has
  // changed, the delegate is notified.
  void UpdateStarredStateForCurrentURL();

  // Whether the current URL is starred.
  bool is_starred_;

  // Registers and unregisters us for notifications.
  NotificationRegistrar registrar_;

  // Owning TabContentsWrapper.
  TabContentsWrapper* tab_contents_wrapper_;

  // Delegate for notifying our owner (usually Browser) about stuff. Not owned
  // by us.
  BookmarksTabHelperDelegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(BookmarksTabHelper);
};

#endif  // CHROME_BROWSER_UI_BOOKMARKS_BOOKMARKS_TAB_HELPER_H_
