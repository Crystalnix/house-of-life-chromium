// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SSL_SSL_BLOCKING_PAGE_H_
#define CHROME_BROWSER_SSL_SSL_BLOCKING_PAGE_H_
#pragma once

#include <string>
#include <vector>

#include "base/string16.h"
#include "chrome/browser/ssl/ssl_error_info.h"
#include "chrome/browser/tab_contents/chrome_interstitial_page.h"

class DictionaryValue;
class SSLCertErrorHandler;

// This class is responsible for showing/hiding the interstitial page that is
// shown when a certificate error happens.
// It deletes itself when the interstitial page is closed.
class SSLBlockingPage : public ChromeInterstitialPage {
 public:
  // An interface that classes that want to interact with the SSLBlockingPage
  // should implement.
  class Delegate {
   public:
    // Should return the information about the error that causes this blocking
    // page.
    virtual SSLErrorInfo GetSSLErrorInfo(SSLCertErrorHandler* handler) = 0;

    // Notification that the user chose to reject the certificate.
    virtual void OnDenyCertificate(SSLCertErrorHandler* handler) = 0;

    // Notification that the user chose to accept the certificate.
    virtual void OnAllowCertificate(SSLCertErrorHandler* handler) = 0;

   protected:
    virtual ~Delegate() {}
  };

  // The severity of the certificate error.
  enum ErrorLevel {
    ERROR_OVERRIDABLE,  // The interstitial page has a "Proceed anyway" button.
    ERROR_FATAL,        // The interstitial page doesn't allow the user to
                        // proceed to the site.
  };

  SSLBlockingPage(SSLCertErrorHandler* handler, Delegate* delegate,
                  ErrorLevel error_level);
  virtual ~SSLBlockingPage();

  // A method that sets strings in the specified dictionary from the passed
  // vector so that they can be used to resource the ssl_roadblock.html/
  // ssl_error.html files.
  // Note: there can be up to 5 strings in |extra_info|.
  static void SetExtraInfo(DictionaryValue* strings,
                           const std::vector<string16>& extra_info);

 protected:
  // ChromeInterstitialPage implementation.
  virtual std::string GetHTMLContents();
  virtual void CommandReceived(const std::string& command);
  virtual void UpdateEntry(NavigationEntry* entry);
  virtual void Proceed();
  virtual void DontProceed();

 private:
  void NotifyDenyCertificate();
  void NotifyAllowCertificate();

  // The error we represent.  We will either call CancelRequest() or
  // ContinueRequest() on this object.
  scoped_refptr<SSLCertErrorHandler> handler_;

  // Our delegate.  It provides useful information, like the title and details
  // about this error.
  Delegate* delegate_;

  // A flag to indicate if we've notified |delegate_| of the user's decision.
  bool delegate_has_been_notified_;

  // Is the certificate error overridable or fatal?
  ErrorLevel error_level_;

  DISALLOW_COPY_AND_ASSIGN(SSLBlockingPage);
};

#endif  // CHROME_BROWSER_SSL_SSL_BLOCKING_PAGE_H_
