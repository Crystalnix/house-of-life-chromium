// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_THUNK_RESOURCE_CREATION_API_H_
#define PPAPI_THUNK_RESOURCE_CREATION_API_H_

#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/ppb_image_data.h"
#include "ppapi/proxy/interface_id.h"

struct PP_Size;

namespace ppapi {
namespace thunk {

// A functional API for creating resource types. Separating out the creation
// functions here allows us to implement most resources as a pure "resource
// API", meaning all calls are routed on a per-resource-object basis. The
// creationg functions are not per-object (since there's no object during
// creation) so need functional routing based on the instance ID.
class ResourceCreationAPI {
 public:
  static const ::pp::proxy::InterfaceID interface_id =
      ::pp::proxy::INTERFACE_ID_RESOURCE_CREATION;

  virtual PP_Resource CreateGraphics2D(PP_Instance instance,
                                       const PP_Size& size,
                                       PP_Bool is_always_opaque) = 0;
  virtual PP_Resource CreateImageData(PP_Instance instance,
                                      PP_ImageDataFormat format,
                                      const PP_Size& size,
                                      PP_Bool init_to_zero) = 0;
};

}  // namespace thunk
}  // namespace ppapi

#endif  // PPAPI_THUNK_RESOURCE_CREATION_API_H_
