// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "printing/printing_context_mac.h"

#import <ApplicationServices/ApplicationServices.h>
#import <AppKit/AppKit.h>

#include "base/logging.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/sys_string_conversions.h"
#include "base/values.h"
#include "printing/print_settings_initializer_mac.h"

static const CFStringRef kColorModel = CFSTR("ColorModel");
static const CFStringRef kGrayColor = CFSTR("Gray");

namespace printing {

// static
PrintingContext* PrintingContext::Create(const std::string& app_locale) {
  return static_cast<PrintingContext*>(new PrintingContextMac(app_locale));
}

PrintingContextMac::PrintingContextMac(const std::string& app_locale)
    : PrintingContext(app_locale),
      context_(NULL) {
}

PrintingContextMac::~PrintingContextMac() {
  ReleaseContext();
}

void PrintingContextMac::AskUserForSettings(gfx::NativeView parent_view,
                                            int max_pages,
                                            bool has_selection,
                                            PrintSettingsCallback* callback) {
  DCHECK([NSThread isMainThread]);

  // We deliberately don't feed max_pages into the dialog, because setting
  // NSPrintLastPage makes the print dialog pre-select the option to only print
  // a range.

  // TODO(stuartmorgan): implement 'print selection only' (probably requires
  // adding a new custom view to the panel on 10.5; 10.6 has
  // NSPrintPanelShowsPrintSelection).
  NSPrintPanel* panel = [NSPrintPanel printPanel];
  NSPrintInfo* printInfo = [NSPrintInfo sharedPrintInfo];

  NSPrintPanelOptions options = [panel options];
  options |= NSPrintPanelShowsPaperSize;
  options |= NSPrintPanelShowsOrientation;
  options |= NSPrintPanelShowsScaling;
  [panel setOptions:options];

  // Set the print job title text.
  if (parent_view) {
    NSString* job_title = [[parent_view window] title];
    if (job_title) {
      PMPrintSettings printSettings =
          (PMPrintSettings)[printInfo PMPrintSettings];
      PMPrintSettingsSetJobName(printSettings, (CFStringRef)job_title);
      [printInfo updateFromPMPrintSettings];
    }
  }

  // TODO(stuartmorgan): We really want a tab sheet here, not a modal window.
  // Will require restructuring the PrintingContext API to use a callback.
  NSInteger selection = [panel runModalWithPrintInfo:printInfo];
  if (selection == NSOKButton) {
    ParsePrintInfo([panel printInfo]);
    callback->Run(OK);
  } else {
    callback->Run(CANCEL);
  }
}

PrintingContext::Result PrintingContextMac::UseDefaultSettings() {
  DCHECK(!in_print_job_);

  ParsePrintInfo([NSPrintInfo sharedPrintInfo]);

  return OK;
}

PrintingContext::Result PrintingContextMac::UpdatePrintSettings(
    const DictionaryValue& job_settings, const PageRanges& ranges) {
  DCHECK(!in_print_job_);

  ResetSettings();
  print_info_.reset([[NSPrintInfo sharedPrintInfo] copy]);

  bool collate;
  bool color;
  bool landscape;
  bool print_to_pdf;
  int copies;
  int duplex_mode;
  std::string device_name;

  if (!job_settings.GetBoolean(kSettingLandscape, &landscape) ||
      !job_settings.GetBoolean(kSettingCollate, &collate) ||
      !job_settings.GetBoolean(kSettingColor, &color) ||
      !job_settings.GetBoolean(kSettingPrintToPDF, &print_to_pdf) ||
      !job_settings.GetInteger(kSettingDuplexMode, &duplex_mode) ||
      !job_settings.GetInteger(kSettingCopies, &copies) ||
      !job_settings.GetString(kSettingDeviceName, &device_name)) {
    return OnError();
  }

  if (!print_to_pdf) {
    if (!SetPrinter(device_name))
      return OnError();

    if (!SetCopiesInPrintSettings(copies))
      return OnError();

    if (!SetCollateInPrintSettings(collate))
      return OnError();

    if (!SetDuplexModeInPrintSettings(
            static_cast<DuplexMode>(duplex_mode))) {
      return OnError();
    }

    if (!SetOutputIsColor(color))
      return OnError();
  }

  if (!SetOrientationIsLandscape(landscape))
    return OnError();

  [print_info_.get() updateFromPMPrintSettings];

  InitPrintSettingsFromPrintInfo(ranges);
  return OK;
}

void PrintingContextMac::InitPrintSettingsFromPrintInfo(
    const PageRanges& ranges) {
  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);
  PMPageFormat page_format =
      static_cast<PMPageFormat>([print_info_.get() PMPageFormat]);
  PMPrinter printer;
  PMSessionGetCurrentPrinter(print_session, &printer);
  PrintSettingsInitializerMac::InitPrintSettings(
      printer, page_format, ranges, false, &settings_);
}

bool PrintingContextMac::SetPrinter(const std::string& device_name) {
  DCHECK(print_info_.get());
  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);

  PMPrinter current_printer;
  if (PMSessionGetCurrentPrinter(print_session, &current_printer) != noErr)
    return false;

  CFStringRef current_printer_id = PMPrinterGetID(current_printer);
  if (!current_printer_id)
    return false;

  base::mac::ScopedCFTypeRef<CFStringRef> new_printer_id(
      base::SysUTF8ToCFStringRef(device_name));
  if (!new_printer_id.get())
    return false;

  if (CFStringCompare(new_printer_id.get(), current_printer_id, 0) ==
          kCFCompareEqualTo) {
    return true;
  }

  PMPrinter new_printer = PMPrinterCreateFromPrinterID(new_printer_id.get());
  if (new_printer == NULL)
    return false;

  OSStatus status = PMSessionSetCurrentPMPrinter(print_session, new_printer);
  PMRelease(new_printer);
  return status == noErr;
}

bool PrintingContextMac::SetCopiesInPrintSettings(int copies) {
  if (copies < 1)
    return false;

  PMPrintSettings pmPrintSettings =
      static_cast<PMPrintSettings>([print_info_.get() PMPrintSettings]);
  return PMSetCopies(pmPrintSettings, copies, false) == noErr;
}

bool PrintingContextMac::SetCollateInPrintSettings(bool collate) {
  PMPrintSettings pmPrintSettings =
      static_cast<PMPrintSettings>([print_info_.get() PMPrintSettings]);
  return PMSetCollate(pmPrintSettings, collate) == noErr;
}

bool PrintingContextMac::SetOrientationIsLandscape(bool landscape) {
  PMPageFormat page_format =
      static_cast<PMPageFormat>([print_info_.get() PMPageFormat]);

  PMOrientation orientation = landscape ? kPMLandscape : kPMPortrait;

  if (PMSetOrientation(page_format, orientation, false) != noErr)
    return false;

  [print_info_.get() updateFromPMPageFormat];
  return true;
}

bool PrintingContextMac::SetDuplexModeInPrintSettings(DuplexMode mode) {
  PMDuplexMode duplexSetting;
  switch (mode) {
    case LONG_EDGE:
      duplexSetting = kPMDuplexNoTumble;
      break;
    case SHORT_EDGE:
      duplexSetting = kPMDuplexTumble;
      break;
    default:
      duplexSetting = kPMDuplexNone;
      break;
  }

  PMPrintSettings pmPrintSettings =
      static_cast<PMPrintSettings>([print_info_.get() PMPrintSettings]);
  return PMSetDuplex(pmPrintSettings, duplexSetting) == noErr;
}

bool PrintingContextMac::SetOutputIsColor(bool color) {
  PMPrintSettings pmPrintSettings =
      static_cast<PMPrintSettings>([print_info_.get() PMPrintSettings]);
  CFStringRef output_color = color ? NULL : kGrayColor;

  return PMPrintSettingsSetValue(pmPrintSettings,
                                 kColorModel,
                                 output_color,
                                 false) == noErr;
}

void PrintingContextMac::ParsePrintInfo(NSPrintInfo* print_info) {
  ResetSettings();
  print_info_.reset([print_info retain]);
  PageRanges page_ranges;
  NSDictionary* print_info_dict = [print_info_.get() dictionary];
  if (![[print_info_dict objectForKey:NSPrintAllPages] boolValue]) {
    PageRange range;
    range.from = [[print_info_dict objectForKey:NSPrintFirstPage] intValue] - 1;
    range.to = [[print_info_dict objectForKey:NSPrintLastPage] intValue] - 1;
    page_ranges.push_back(range);
  }
  InitPrintSettingsFromPrintInfo(page_ranges);
}

PrintingContext::Result PrintingContextMac::InitWithSettings(
    const PrintSettings& settings) {
  DCHECK(!in_print_job_);

  settings_ = settings;

  NOTIMPLEMENTED();

  return FAILED;
}

PrintingContext::Result PrintingContextMac::NewDocument(
    const string16& document_name) {
  DCHECK(!in_print_job_);

  in_print_job_ = true;

  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);
  PMPrintSettings print_settings =
      static_cast<PMPrintSettings>([print_info_.get() PMPrintSettings]);
  PMPageFormat page_format =
      static_cast<PMPageFormat>([print_info_.get() PMPageFormat]);

  base::mac::ScopedCFTypeRef<CFStringRef> job_title(
      base::SysUTF16ToCFStringRef(document_name));
  PMPrintSettingsSetJobName(print_settings, job_title.get());

  OSStatus status = PMSessionBeginCGDocumentNoDialog(print_session,
                                                     print_settings,
                                                     page_format);
  if (status != noErr)
    return OnError();

  return OK;
}

PrintingContext::Result PrintingContextMac::NewPage() {
  if (abort_printing_)
    return CANCEL;
  DCHECK(in_print_job_);
  DCHECK(!context_);

  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);
  PMPageFormat page_format =
      static_cast<PMPageFormat>([print_info_.get() PMPageFormat]);
  OSStatus status;
  status = PMSessionBeginPageNoDialog(print_session, page_format, NULL);
  if (status != noErr)
    return OnError();
  status = PMSessionGetCGGraphicsContext(print_session, &context_);
  if (status != noErr)
    return OnError();

  return OK;
}

PrintingContext::Result PrintingContextMac::PageDone() {
  if (abort_printing_)
    return CANCEL;
  DCHECK(in_print_job_);
  DCHECK(context_);

  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);
  OSStatus status = PMSessionEndPageNoDialog(print_session);
  if (status != noErr)
    OnError();
  context_ = NULL;

  return OK;
}

PrintingContext::Result PrintingContextMac::DocumentDone() {
  if (abort_printing_)
    return CANCEL;
  DCHECK(in_print_job_);

  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);
  OSStatus status = PMSessionEndDocumentNoDialog(print_session);
  if (status != noErr)
    OnError();

  ResetSettings();
  return OK;
}

void PrintingContextMac::Cancel() {
  abort_printing_ = true;
  in_print_job_ = false;
  context_ = NULL;

  PMPrintSession print_session =
      static_cast<PMPrintSession>([print_info_.get() PMPrintSession]);
  PMSessionEndPageNoDialog(print_session);
}

void PrintingContextMac::ReleaseContext() {
  print_info_.reset();
  context_ = NULL;
}

gfx::NativeDrawingContext PrintingContextMac::context() const {
  return context_;
}

}  // namespace printing
