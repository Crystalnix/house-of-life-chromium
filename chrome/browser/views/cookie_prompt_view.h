// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_VIEWS_COOKIE_PROMPT_VIEW_H_
#define CHROME_BROWSER_VIEWS_COOKIE_PROMPT_VIEW_H_

#include <string>

#include "base/task.h"
#include "chrome/browser/browsing_data_local_storage_helper.h"
#include "chrome/browser/cookie_prompt_modal_dialog_delegate.h"
#include "chrome/browser/views/cookie_info_view.h"
#include "chrome/browser/views/modal_dialog_delegate.h"
#include "googleurl/src/gurl.h"
#include "views/controls/button/button.h"
#include "views/controls/link.h"
#include "views/view.h"
#include "views/window/dialog_delegate.h"
#include "views/window/window.h"

namespace views {
class NativeButton;
class RadioButton;
}

class CookieInfoView;
class CookiePromptModalDialog;
class LocalStorageInfoView;
class Profile;
class Timer;

// Cookie alert dialog UI.
class CookiePromptView : public views::View,
                          public ModalDialogDelegate,
                          public views::ButtonListener,
                          public views::LinkController,
                          public CookieInfoViewDelegate {
 public:
  // Show the Cookies Window, creating one if necessary.
  CookiePromptView(
      CookiePromptModalDialog* parent,
      gfx::NativeWindow root_window,
      Profile* profile,
      const GURL& url,
      const std::string& cookie_line,
      CookiePromptModalDialogDelegate* delegate);

  CookiePromptView(
      CookiePromptModalDialog* parent,
      gfx::NativeWindow root_window,
      Profile* profile,
      const BrowsingDataLocalStorageHelper::LocalStorageInfo&
          local_storage_info,
      CookiePromptModalDialogDelegate* delegate);

  virtual ~CookiePromptView();

 protected:
  // views::View overrides.
  virtual gfx::Size GetPreferredSize();
  virtual void ViewHierarchyChanged(bool is_add,
                                    views::View* parent,
                                    views::View* child);

  // ModalDialogDelegate overrides.
  virtual gfx::NativeWindow GetDialogRootWindow();

  // views::DialogDelegate overrides.
  virtual bool CanResize() const { return false; }
  virtual std::wstring GetWindowTitle() const;
  virtual void WindowClosing();
  virtual views::View* GetContentsView();
  virtual bool IsModal() const { return true; }

  // views::ButtonListener overrides.
  virtual void ButtonPressed(views::Button* sender, const views::Event& event);

  // views::LinkController overrides.
  virtual void LinkActivated(views::Link* source, int event_flags);

  // views::WindowDelegate overrides.
  virtual int GetDialogButtons() const {
    return MessageBoxFlags::DIALOGBUTTON_NONE;
  }

  // CookieInfoViewDelegate overrides:
  virtual void ModifyExpireDate(bool session_expire);

 private:
  // Use the static factory method to show.
  explicit CookiePromptView(Profile* profile,
                             CookiePromptModalDialogDelegate* delegate);

  // Initialize the dialog layout.
  void Init();

  // Shows or hides cooke info view and changes parent.
  void ToggleDetailsViewExpand();

  // Calculates view size offset depending on visibility of cookie details.
  int GetExtendedViewHeight();

  // Initializes text resources needed to display this view.
  void InitializeViewResources(const std::string& domain);

  views::RadioButton* remember_radio_;
  views::RadioButton* ask_radio_;
  views::NativeButton* allow_button_;
  views::NativeButton* block_button_;
  views::Link* show_cookie_link_;
  views::Link* manage_cookies_link_;
  views::View* info_view_;

  // True if cookie should expire with this session.
  bool session_expire_;

  // True if cookie info view is currently shown and window expanded.
  bool expanded_view_;

  // True if the outcome of this dialog has been signaled to the delegate.
  bool signaled_;

  // Prompt window title.
  std::wstring title_;

  // Whether we're showing cookie UI as opposed to other site data.
  bool cookie_ui_;

  // A pointer to the AppModalDialog that owns us.
  CookiePromptModalDialog* parent_;

  gfx::NativeWindow root_window_;

  // The Profile for which Cookies are displayed.
  Profile* profile_;

  // Cookie / local storage domain.
  std::string domain_;

  // Domain name formatted for displaying (removed leading '.').
  std::wstring display_domain_;

  // Displayed cookie. Only used when |cookie_ui_| is true.
  std::string cookie_line_;

  // Displayed local storage info.  Only used when |cookie_ui_| is false.
  BrowsingDataLocalStorageHelper::LocalStorageInfo local_storage_info_;

  CookiePromptModalDialogDelegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(CookiePromptView);
};

#endif  // CHROME_BROWSER_VIEWS_COOKIE_PROMPT_VIEW_H_

