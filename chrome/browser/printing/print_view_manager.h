// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PRINTING_PRINT_VIEW_MANAGER_H_
#define CHROME_BROWSER_PRINTING_PRINT_VIEW_MANAGER_H_
#pragma once

#include "base/memory/ref_counted.h"
#include "base/string16.h"
#include "content/browser/tab_contents/tab_contents_observer.h"
#include "content/common/notification_observer.h"
#include "content/common/notification_registrar.h"
#include "printing/printed_pages_source.h"

class RenderViewHost;
class TabContents;
struct PrintHostMsg_DidPrintPage_Params;

namespace printing {

class JobEventDetails;
class PrintJob;
class PrintJobWorkerOwner;

// Manages the print commands in relation to a TabContents. TabContents
// delegates a few printing related commands to this instance.
class PrintViewManager : public NotificationObserver,
                         public PrintedPagesSource,
                         public TabContentsObserver {
 public:
  explicit PrintViewManager(TabContents* tab_contents);
  virtual ~PrintViewManager();

  // Override the title for this PrintViewManager's PrintJobs using the title
  // in |tab_contents|.
  void OverrideTitle(TabContents* tab_contents);

  // Prints the current document immediately. Since the rendering is
  // asynchronous, the actual printing will not be completed on the return of
  // this function. Returns false if printing is impossible at the moment.
  bool PrintNow();

  // PrintedPagesSource implementation.
  virtual string16 RenderSourceName();
  virtual GURL RenderSourceUrl();

  // NotificationObserver implementation.
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

  // TabContentsObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message);

  // Terminates or cancels the print job if one was pending.
  virtual void RenderViewGone();

  // Cancels the print job.
  virtual void StopNavigation();

 private:
  // IPC Message handlers.
  void OnDidGetPrintedPagesCount(int cookie, int number_pages);
  void OnDidPrintPage(const PrintHostMsg_DidPrintPage_Params& params);
  void OnPrintingFailed(int cookie);

  // Processes a NOTIFY_PRINT_JOB_EVENT notification.
  void OnNotifyPrintJobEvent(const JobEventDetails& event_details);

  // Requests the RenderView to render all the missing pages for the print job.
  // Noop if no print job is pending. Returns true if at least one page has been
  // requested to the renderer.
  bool RenderAllMissingPagesNow();

  // Quits the current message loop if these conditions hold true: a document is
  // loaded and is complete and waiting_for_pages_to_be_rendered_ is true. This
  // function is called in DidPrintPage() or on ALL_PAGES_REQUESTED
  // notification. The inner message loop is created was created by
  // RenderAllMissingPagesNow().
  void ShouldQuitFromInnerMessageLoop();

  // Creates a new empty print job. It has no settings loaded. If there is
  // currently a print job, safely disconnect from it. Returns false if it is
  // impossible to safely disconnect from the current print job or it is
  // impossible to create a new print job.
  bool CreateNewPrintJob(PrintJobWorkerOwner* job);

  // Makes sure the current print_job_ has all its data before continuing, and
  // disconnect from it.
  void DisconnectFromCurrentPrintJob();

  // Notify that the printing is done.
  void PrintingDone(bool success);

  // Terminates the print job. Noop if no print job has been created. If
  // |cancel| is true, cancel it instead of waiting for the job to finish. Will
  // call ReleasePrintJob().
  void TerminatePrintJob(bool cancel);

  // Releases print_job_. Correctly deregisters from notifications. Noop if
  // no print job has been created.
  void ReleasePrintJob();

  // Runs an inner message loop. It will set inside_inner_message_loop_ to true
  // while the blocking inner message loop is running. This is useful in cases
  // where the RenderView is about to be destroyed while a printing job isn't
  // finished.
  bool RunInnerMessageLoop();

  // In the case of Scripted Printing, where the renderer is controlling the
  // control flow, print_job_ is initialized whenever possible. No-op is
  // print_job_ is initialized.
  bool OpportunisticallyCreatePrintJob(int cookie);

  NotificationRegistrar registrar_;

  // Manages the low-level talk to the printer.
  scoped_refptr<PrintJob> print_job_;

  // Number of pages to print in the print job.
  int number_pages_;

  // Indication of success of the print job.
  bool printing_succeeded_;

  // Running an inner message loop inside RenderAllMissingPagesNow(). This means
  // we are _blocking_ until all the necessary pages have been rendered or the
  // print settings are being loaded.
  bool inside_inner_message_loop_;

#if defined(OS_POSIX) && !defined(OS_MACOSX)
  // Set to true when OnDidPrintPage() should be expecting the first page.
  bool expecting_first_page_;
#endif

  // Title override.
  bool is_title_overridden_;
  string16 overridden_title_;

  DISALLOW_COPY_AND_ASSIGN(PrintViewManager);
};

}  // namespace printing

#endif  // CHROME_BROWSER_PRINTING_PRINT_VIEW_MANAGER_H_
