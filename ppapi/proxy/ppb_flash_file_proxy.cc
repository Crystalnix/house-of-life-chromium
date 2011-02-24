// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ppapi/proxy/ppb_flash_file_proxy.h"

#include "base/logging.h"
#include "build/build_config.h"
#include "ppapi/c/dev/pp_file_info_dev.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/private/ppb_flash_file.h"
#include "ppapi/proxy/plugin_dispatcher.h"
#include "ppapi/proxy/plugin_resource.h"
#include "ppapi/proxy/ppapi_messages.h"

namespace pp {
namespace proxy {

namespace {

// Given an error code and a handle result from a Pepper API call, converts
// to a PlatformFileForTransit, possibly also updating the error value if
// an error occurred.
IPC::PlatformFileForTransit PlatformFileToPlatformFileForTransit(
    int32_t* error,
    base::PlatformFile file) {
  if (*error != PP_OK)
    return IPC::InvalidPlatformFileForTransit();
#if defined(OS_WIN)
/* TODO(brettw): figure out how to get the target process handle.
  HANDLE result;
  if (!::DuplicateHandle(::GetCurrentProcess(), file,
                         target_process, &result, 0, false,
                         DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)) {
    *error = PP_ERROR_NOACCESS;
    return INVALID_HANDLE_VALUE;
  }
  return result;
*/
  NOTIMPLEMENTED();
  *error = PP_ERROR_NOACCESS;
  return INVALID_HANDLE_VALUE;
#elif defined(OS_POSIX)
  return base::FileDescriptor(file, true);
#endif
}

void FreeDirContents(PP_Instance /* instance */,
                     PP_DirContents_Dev* contents) {
  for (int32_t i = 0; i < contents->count; ++i)
    delete[] contents->entries[i].name;
  delete[] contents->entries;
  delete contents;
}

int32_t OpenModuleLocalFile(PP_Instance instance,
                            const char* path,
                            int32_t mode,
                            PP_FileHandle* file) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return PP_ERROR_BADARGUMENT;

  int32_t result = PP_ERROR_FAILED;
  IPC::PlatformFileForTransit transit;
  dispatcher->Send(new PpapiHostMsg_PPBFlashFile_ModuleLocal_OpenFile(
      INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL,
      instance, path, mode, &transit, &result));
  *file = IPC::PlatformFileForTransitToPlatformFile(transit);
  return result;
}

int32_t RenameModuleLocalFile(PP_Instance instance,
                              const char* path_from,
                              const char* path_to) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return PP_ERROR_BADARGUMENT;

  int32_t result = PP_ERROR_FAILED;
  dispatcher->Send(new PpapiHostMsg_PPBFlashFile_ModuleLocal_RenameFile(
      INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL,
      instance, path_from, path_to, &result));
  return result;
}

int32_t DeleteModuleLocalFileOrDir(PP_Instance instance,
                                   const char* path,
                                   PP_Bool recursive) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return PP_ERROR_BADARGUMENT;

  int32_t result = PP_ERROR_FAILED;
  dispatcher->Send(new PpapiHostMsg_PPBFlashFile_ModuleLocal_DeleteFileOrDir(
      INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL,
      instance, path, recursive, &result));
  return result;
}

int32_t CreateModuleLocalDir(PP_Instance instance, const char* path) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return PP_ERROR_BADARGUMENT;

  int32_t result = PP_ERROR_FAILED;
  dispatcher->Send(new PpapiHostMsg_PPBFlashFile_ModuleLocal_CreateDir(
      INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL, instance, path, &result));
  return result;
}

int32_t QueryModuleLocalFile(PP_Instance instance,
                             const char* path,
                             PP_FileInfo_Dev* info) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return PP_ERROR_BADARGUMENT;

  int32_t result = PP_ERROR_FAILED;
  dispatcher->Send(new PpapiHostMsg_PPBFlashFile_ModuleLocal_QueryFile(
      INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL, instance, path, info, &result));
  return result;
}

int32_t GetModuleLocalDirContents(PP_Instance instance,
                                  const char* path,
                                  PP_DirContents_Dev** contents) {
  PluginDispatcher* dispatcher = PluginDispatcher::GetForInstance(instance);
  if (!dispatcher)
    return PP_ERROR_BADARGUMENT;

  int32_t result = PP_ERROR_FAILED;
  std::vector<SerializedDirEntry> entries;
  dispatcher->Send(new PpapiHostMsg_PPBFlashFile_ModuleLocal_GetDirContents(
      INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL,
      instance, path, &entries, &result));

  if (result != PP_OK)
    return result;

  // Copy the serialized dir entries to the output struct.
  *contents = new PP_DirContents_Dev;
  (*contents)->count = static_cast<int32_t>(entries.size());
  (*contents)->entries = new PP_DirEntry_Dev[entries.size()];
  for (size_t i = 0; i < entries.size(); i++) {
    const SerializedDirEntry& source = entries[i];
    PP_DirEntry_Dev* dest = &(*contents)->entries[i];

    char* name_copy = new char[source.name.size() + 1];
    memcpy(name_copy, source.name.c_str(), source.name.size() + 1);
    dest->name = name_copy;
    dest->is_dir = BoolToPPBool(source.is_dir);
  }

  return result;
}

const PPB_Flash_File_ModuleLocal flash_file_module_local_interface = {
  &OpenModuleLocalFile,
  &RenameModuleLocalFile,
  &DeleteModuleLocalFileOrDir,
  &CreateModuleLocalDir,
  &QueryModuleLocalFile,
  &GetModuleLocalDirContents,
  &FreeDirContents,
};

InterfaceProxy* CreateFlashFileModuleLocalProxy(Dispatcher* dispatcher,
                                                const void* target_interface) {
  return new PPB_Flash_File_ModuleLocal_Proxy(dispatcher, target_interface);
}

}  // namespace

PPB_Flash_File_ModuleLocal_Proxy::PPB_Flash_File_ModuleLocal_Proxy(
    Dispatcher* dispatcher,
    const void* target_interface)
    : InterfaceProxy(dispatcher, target_interface) {
}

PPB_Flash_File_ModuleLocal_Proxy::~PPB_Flash_File_ModuleLocal_Proxy() {
}

// static
const InterfaceProxy::Info* PPB_Flash_File_ModuleLocal_Proxy::GetInfo() {
  static const Info info = {
    &flash_file_module_local_interface,
    PPB_FLASH_FILE_MODULELOCAL_INTERFACE,
    INTERFACE_ID_PPB_FLASH_FILE_MODULELOCAL,
    true,
    &CreateFlashFileModuleLocalProxy,
  };
  return &info;
}

bool PPB_Flash_File_ModuleLocal_Proxy::OnMessageReceived(
    const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(PPB_Flash_File_ModuleLocal_Proxy, msg)
    IPC_MESSAGE_HANDLER(PpapiHostMsg_PPBFlashFile_ModuleLocal_OpenFile,
                        OnMsgOpenFile)
    IPC_MESSAGE_HANDLER(PpapiHostMsg_PPBFlashFile_ModuleLocal_RenameFile,
                        OnMsgRenameFile)
    IPC_MESSAGE_HANDLER(PpapiHostMsg_PPBFlashFile_ModuleLocal_DeleteFileOrDir,
                        OnMsgDeleteFileOrDir)
    IPC_MESSAGE_HANDLER(PpapiHostMsg_PPBFlashFile_ModuleLocal_CreateDir,
                        OnMsgCreateDir)
    IPC_MESSAGE_HANDLER(PpapiHostMsg_PPBFlashFile_ModuleLocal_QueryFile,
                        OnMsgQueryFile)
    IPC_MESSAGE_HANDLER(PpapiHostMsg_PPBFlashFile_ModuleLocal_GetDirContents,
                        OnMsgGetDirContents)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  // TODO(brettw) handle bad messages!
  return handled;
}

void PPB_Flash_File_ModuleLocal_Proxy::OnMsgOpenFile(
    PP_Instance instance,
    const std::string& path,
    int32_t mode,
    IPC::PlatformFileForTransit* file_handle,
    int32_t* result) {
  base::PlatformFile file;
  *result = ppb_flash_file_module_local_target()->
      OpenFile(instance, path.c_str(), mode, &file);
  *file_handle = PlatformFileToPlatformFileForTransit(result, file);
}

void PPB_Flash_File_ModuleLocal_Proxy::OnMsgRenameFile(
    PP_Instance instance,
    const std::string& path_from,
    const std::string& path_to,
    int32_t* result) {
  *result = ppb_flash_file_module_local_target()->
      RenameFile(instance, path_from.c_str(), path_to.c_str());
}

void PPB_Flash_File_ModuleLocal_Proxy::OnMsgDeleteFileOrDir(
    PP_Instance instance,
    const std::string& path,
    PP_Bool recursive,
    int32_t* result) {
  *result = ppb_flash_file_module_local_target()->
      DeleteFileOrDir(instance, path.c_str(), recursive);
}

void PPB_Flash_File_ModuleLocal_Proxy::OnMsgCreateDir(PP_Instance instance,
                                                      const std::string& path,
                                                      int32_t* result) {
  *result = ppb_flash_file_module_local_target()->
      CreateDir(instance, path.c_str());
}

void PPB_Flash_File_ModuleLocal_Proxy::OnMsgQueryFile(PP_Instance instance,
                                                      const std::string& path,
                                                      PP_FileInfo_Dev* info,
                                                      int32_t* result) {
  *result = ppb_flash_file_module_local_target()->
      QueryFile(instance, path.c_str(), info);
}

void PPB_Flash_File_ModuleLocal_Proxy::OnMsgGetDirContents(
    PP_Instance instance,
    const std::string& path,
    std::vector<pp::proxy::SerializedDirEntry>* entries,
    int32_t* result) {
  PP_DirContents_Dev* contents = NULL;
  *result = ppb_flash_file_module_local_target()->
      GetDirContents(instance, path.c_str(), &contents);
  if (*result != PP_OK)
    return;

  // Convert the list of entries to the serialized version.
  entries->resize(contents->count);
  for (int32_t i = 0; i < contents->count; i++) {
    (*entries)[i].name.assign(contents->entries[i].name);
    (*entries)[i].is_dir = PPBoolToBool(contents->entries[i].is_dir);
  }
  ppb_flash_file_module_local_target()->FreeDirContents(instance, contents);
}

}  // namespace proxy
}  // namespace pp