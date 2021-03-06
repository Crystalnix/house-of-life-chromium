// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windows.h>
#include <winspool.h>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/process_util.h"
#include "base/win/scoped_handle.h"
#include "base/win/windows_version.h"
#include "cloud_print/virtual_driver/win/virtual_driver_consts.h"
#include "cloud_print/virtual_driver/win/virtual_driver_helpers.h"

namespace {

bool IsSystem64Bit() {
  base::win::OSInfo::WindowsArchitecture arch =
      base::win::OSInfo::GetInstance()->architecture();
  return (arch == base::win::OSInfo::X64_ARCHITECTURE) ||
         (arch == base::win::OSInfo::IA64_ARCHITECTURE);
}

HRESULT GetGpdPath(FilePath* path) {
  if (!PathService::Get(base::DIR_EXE, path)) {
    LOG(ERROR) << "Unable to get install path.";
    return ERROR_PATH_NOT_FOUND;
  }
  *path = path->Append(L"gcp.gpd");
  return S_OK;
}

const wchar_t *GetPortMonitorDllName() {
  if (IsSystem64Bit()) {
    return cloud_print::kPortMonitorDllName64;
  } else {
    return cloud_print::kPortMonitorDllName32;
  }
}

HRESULT GetPortMonitorDllPath(FilePath* path) {
  if (!PathService::Get(base::DIR_EXE, path)) {
    LOG(ERROR) << "Unable to get install path.";
    return ERROR_PATH_NOT_FOUND;
  }
  *path = path->Append(GetPortMonitorDllName());
  return S_OK;
}

HRESULT GetPortMonitorInstallPath(FilePath* path) {
  if (IsSystem64Bit()) {
    if (!PathService::Get(base::DIR_WINDOWS, path)) {
      return ERROR_PATH_NOT_FOUND;
    }
    // Sysnative will bypass filesystem redirection and give us
    // the location of the 64bit system32 from a 32 bit process.
    *path = path->Append(L"sysnative");
  } else {
    if (!PathService::Get(base::DIR_SYSTEM, path)) {
      LOG(ERROR) << "Unable to get system path.";
      return ERROR_PATH_NOT_FOUND;
    }
  }
  *path = path->Append(GetPortMonitorDllName());
  return S_OK;
}

HRESULT GetRegsvr32Path(FilePath* path) {
  if (!PathService::Get(base::DIR_SYSTEM, path)) {
    LOG(ERROR) << "Unable to get system path.";
    return ERROR_PATH_NOT_FOUND;
  }
  *path = path->Append(FilePath(L"regsvr32.exe"));
  return S_OK;
}

HRESULT RegisterPortMonitor(bool install) {
  FilePath target_path;
  HRESULT result = S_OK;
  result = GetPortMonitorInstallPath(&target_path);
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to get port monitor target path.";
    return result;
  }
  FilePath source_path;
  result = GetPortMonitorDllPath(&source_path);
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to get dll source path.";
    return result;
  }
  if (install) {
    if (!file_util::CopyFileW(source_path, target_path)) {
      LOG(ERROR) << "Unable copy port monitor dll from " <<
          source_path.value() << " to " << target_path.value();
      return ERROR_ACCESS_DENIED;
    }
  }
  FilePath regsvr32_path;
  result = GetRegsvr32Path(&regsvr32_path);
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Can't find regsvr32.exe.";
    return result;
  }
  CommandLine command_line(regsvr32_path);
  command_line.AppendArg("/s");
  if (!install) {
    command_line.AppendArg("/u");
  }
  command_line.AppendArgPath(source_path);
  HANDLE process_handle;
  if (!base::LaunchApp(command_line.command_line_string(),
                       true,
                       false,
                       &process_handle)) {
    LOG(ERROR) << "Unable to launch regsvr32.exe.";
    return ERROR_NOT_SUPPORTED;
  }
  base::win::ScopedHandle scoped_process_handle(process_handle);
  DWORD exit_code = S_OK;
  if (!GetExitCodeProcess(scoped_process_handle, &exit_code)) {
    HRESULT result = cloud_print::GetLastHResult();
    LOG(ERROR) << "Unable to get regsvr32.exe exit code.";
    return result;
  }
  if (exit_code != 0) {
    LOG(ERROR) << "Regsvr32.exe failed with " << exit_code;
    return HRESULT_FROM_WIN32(exit_code);
  }
  if (!install) {
    if (!file_util::Delete(target_path, false)) {
      LOG(ERROR) << "Unable to delete " << target_path.value();
      return ERROR_ACCESS_DENIED;
    }
  }
  return S_OK;
}

HRESULT InstallGpd() {
  HRESULT result = S_OK;
  FilePath source_path;
  result = GetGpdPath(&source_path);
  if (!SUCCEEDED(result)) {
    return result;
  }
  FilePath driver_dir;
  cloud_print::GetPrinterDriverDir(&driver_dir);
  FilePath xps_path = driver_dir.Append(L"mxdwdrv.dll");
  FilePath ui_path = driver_dir.Append(L"unidrvui.dll");
  FilePath ui_help_path = driver_dir.Append(L"unidrv.hlp");
  DRIVER_INFO_6 driver_info = {0};
  driver_info.cVersion = 3;
  // None of the print API structures likes constant strings even though they
  // don't modify the string.  const_casting is the cleanest option.
  driver_info.pName = const_cast<LPWSTR>(cloud_print::kVirtualDriverName);
  driver_info.pDriverPath = const_cast<LPWSTR>(xps_path.value().c_str());
  driver_info.pConfigFile = const_cast<LPWSTR>(ui_path.value().c_str());
  driver_info.pDataFile = const_cast<LPWSTR>(source_path.value().c_str());
  driver_info.pHelpFile = const_cast<LPWSTR>(ui_help_path.value().c_str());
  // TODO(abodenha@chromium.org) Pull these strings from resources.
  driver_info.pszMfgName = L"Google";
  driver_info.pszProvider = driver_info.pszMfgName;
  driver_info.pszOEMUrl = L"http://www.google.com/cloudprint";
  driver_info.dwlDriverVersion = 1;
  driver_info.pDefaultDataType = L"RAW";
  // TODO(abodenha@chromium.org) Properly handle dependencies.
  // GPD files are often dependent on various Windows core drivers.
  // I haven't found a reliable way to express those dependencies
  // other than using an INF for installation.
  if (!AddPrinterDriverEx(NULL,
                          6,
                          reinterpret_cast<BYTE*>(&driver_info),
                          APD_COPY_NEW_FILES|APD_COPY_FROM_DIRECTORY)) {
    result = cloud_print::GetLastHResult();
    LOG(ERROR) << "Unable to add printer driver";
    return result;
  }
  return S_OK;
}

HRESULT UninstallGpd() {
  int tries = 10;
  while (!DeletePrinterDriverEx(NULL,
                                NULL,
                                const_cast<LPWSTR>
                                    (cloud_print::kVirtualDriverName),
                                DPD_DELETE_UNUSED_FILES,
                                0) && tries > 0) {
    // After deleting the printer it can take a few seconds before
    // the driver is free for deletion.  Retry a few times before giving up.
    LOG(WARNING) << "Attempt to delete printer driver failed.  Retrying.";
    tries--;
    Sleep(2000);
  }
  if (tries <= 0) {
    HRESULT result = cloud_print::GetLastHResult();
    LOG(ERROR) << "Unable to delete printer driver.";
    return result;
  }
  return S_OK;
}

HRESULT InstallPrinter(void) {
  PRINTER_INFO_2 printer_info = {0};
  printer_info.pPrinterName =
      const_cast<LPWSTR>(cloud_print::kVirtualDriverName);
  printer_info.pPortName = const_cast<LPWSTR>(cloud_print::kPortName);
  printer_info.pDriverName =
      const_cast<LPWSTR>(cloud_print::kVirtualDriverName);
  printer_info.pPrinterName = printer_info.pDriverName;
  // TODO(abodenha@chromium.org) pComment should be localized.
  printer_info.pComment = const_cast<LPWSTR>(cloud_print::kVirtualDriverName);
  printer_info.Attributes = PRINTER_ATTRIBUTE_DIRECT|PRINTER_ATTRIBUTE_LOCAL;
  printer_info.pPrintProcessor = L"winprint";
  printer_info.pDatatype = L"RAW";
  HANDLE handle = AddPrinter(NULL, 2, reinterpret_cast<BYTE*>(&printer_info));
  if (handle == NULL) {
    HRESULT result = cloud_print::GetLastHResult();
    LOG(ERROR) << "Unable to add printer";
    return result;
  }
  ClosePrinter(handle);
  return S_OK;
}

HRESULT UninstallPrinter(void) {
  HANDLE handle = NULL;
  PRINTER_DEFAULTS printer_defaults = {0};
  printer_defaults.DesiredAccess = PRINTER_ALL_ACCESS;
  if (!OpenPrinter(const_cast<LPWSTR>(cloud_print::kVirtualDriverName),
                   &handle,
                   &printer_defaults)) {
    // If we can't open the printer, it was probably already removed.
    LOG(WARNING) << "Unable to open printer";
    return S_OK;
  }
  if (!DeletePrinter(handle)) {
    HRESULT result = cloud_print::GetLastHResult();
    LOG(ERROR) << "Unable to delete printer";
    ClosePrinter(handle);
    return result;
  }
  ClosePrinter(handle);
  return S_OK;
}

HRESULT InstallVirtualDriver(void) {
  HRESULT result = S_OK;
  result = RegisterPortMonitor(true);
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to register port monitor.";
    return result;
  }
  result = InstallGpd();
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to install gpd.";
    return result;
  }
  result = InstallPrinter();
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to install printer.";
    return result;
  }
  return S_OK;
}

HRESULT UninstallVirtualDriver(void) {
  HRESULT result = S_OK;
  result = UninstallPrinter();
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to uninstall gpd.";
    return result;
  }
  result = UninstallGpd();
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to remove gpd.";
    return result;
  }
  result = RegisterPortMonitor(false);
  if (!SUCCEEDED(result)) {
    LOG(ERROR) << "Unable to remove port monitor.";
    return result;
  }
  return S_OK;
}

}  // namespace

int WINAPI WinMain(__in  HINSTANCE hInstance,
            __in  HINSTANCE hPrevInstance,
            __in  LPSTR lpCmdLine,
            __in  int nCmdShow) {
  base::AtExitManager at_exit_manager;
  CommandLine::Init(0, NULL);
  HRESULT retval = S_OK;
  if (CommandLine::ForCurrentProcess()->HasSwitch("uninstall")) {
    retval = UninstallVirtualDriver();
  } else {
    retval = InstallVirtualDriver();
  }
  if (!CommandLine::ForCurrentProcess()->HasSwitch("silent")) {
    cloud_print::DisplayWindowsMessage(NULL, retval);
  }
  return retval;
}

