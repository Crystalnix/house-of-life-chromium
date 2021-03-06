// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_PROXY_RESOURCE_CREATION_PROXY_H_
#define PPAPI_PROXY_RESOURCE_CREATION_PROXY_H_

#include "base/basictypes.h"
#include "ipc/ipc_channel.h"
#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/proxy/serialized_structs.h"
#include "ppapi/shared_impl/function_group_base.h"
#include "ppapi/thunk/resource_creation_api.h"

struct PP_Size;

namespace pp {
namespace proxy {

class HostResource;
class Dispatcher;

class ResourceCreationProxy : public ::ppapi::shared_impl::FunctionGroupBase,
                              public ::ppapi::thunk::ResourceCreationAPI,
                              public ::IPC::Channel::Listener,
                              public IPC::Message::Sender {
 public:
  ResourceCreationProxy(Dispatcher* dispatcher);
  virtual ~ResourceCreationProxy();

  virtual ::ppapi::thunk::ResourceCreationAPI* AsResourceCreation();

  // ResourceCreationAPI (called in plugin).
  virtual PP_Resource CreateGraphics2D(PP_Instance pp_instance,
                                       const PP_Size& size,
                                       PP_Bool is_always_opaque);
  virtual PP_Resource CreateImageData(PP_Instance instance,
                                      PP_ImageDataFormat format,
                                      const PP_Size& size,
                                      PP_Bool init_to_zero);

  virtual bool Send(IPC::Message* msg);
  virtual bool OnMessageReceived(const IPC::Message& msg);

 private:
  // IPC message handlers (called in browser).
  void OnMsgCreateGraphics2D(PP_Instance instance,
                             const PP_Size& size,
                             PP_Bool is_always_opaque,
                             HostResource* result);
  void OnMsgCreateImageData(PP_Instance instance,
                            int32_t format,
                            const PP_Size& size,
                            PP_Bool init_to_zero,
                            HostResource* result,
                            std::string* image_data_desc,
                            ImageHandle* result_image_handle);

  Dispatcher* dispatcher_;

  DISALLOW_COPY_AND_ASSIGN(ResourceCreationProxy);
};

}  // namespace proxy
}  // namespace pp

#endif  // PPAPI_PROXY_RESOURCE_CREATION_PROXY_H_
