// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cloud_print/virtual_driver/win/virtual_driver_consts.h"
#include <windows.h>
#include "cloud_print/virtual_driver/win/virtual_driver_helpers.h"

namespace cloud_print {
const wchar_t kPortMonitorDllName64[] = L"gcp_portmon64.dll";
const wchar_t kPortMonitorDllName32[] = L"gcp_portmon.dll";
const wchar_t kPortName[] = L"GCP:";
const size_t kPortNameSize = sizeof(kPortName);

// The driver name is user visible so it SHOULD be localized, BUT
// the name is used as a key to find both the driver and printer.
// We'll need to be careful.  If the name changes for the
// driver it could create bugs.
const wchar_t kVirtualDriverName[] = L"Google Cloud Print Virtual Printer";
}

