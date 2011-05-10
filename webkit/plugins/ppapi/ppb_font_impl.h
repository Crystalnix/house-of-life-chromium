// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_PLUGINS_PPAPI_PPB_FONT_IMPL_H_
#define WEBKIT_PLUGINS_PPAPI_PPB_FONT_IMPL_H_

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "ppapi/c/dev/ppb_font_dev.h"
#include "ppapi/shared_impl/webkit_forwarding.h"
#include "ppapi/thunk/ppb_font_api.h"
#include "webkit/plugins/ppapi/resource.h"

namespace WebKit {
class WebFont;
}

namespace webkit {
namespace ppapi {

class PluginInstance;

class PPB_Font_Impl : public Resource,
                      public ::ppapi::thunk::PPB_Font_API {
 public:
  PPB_Font_Impl(PluginInstance* instance, const PP_FontDescription_Dev& desc);
  virtual ~PPB_Font_Impl();

  // Returns a pointer to the interface implementing PPB_Font that is exposed to
  // the plugin.
  static const PPB_Font_Dev* GetInterface();

  // ResourceObjectBase.
  virtual ::ppapi::thunk::PPB_Font_API* AsFont_API() OVERRIDE;

  // Resource overrides.
  virtual PPB_Font_Impl* AsPPB_Font_Impl() OVERRIDE;

  // PPB_Font implementation.
  PP_Bool Describe(PP_FontDescription_Dev* description,
                   PP_FontMetrics_Dev* metrics) OVERRIDE;
  PP_Bool DrawTextAt(PP_Resource image_data,
                     const PP_TextRun_Dev* text,
                     const PP_Point* position,
                     uint32_t color,
                     const PP_Rect* clip,
                     PP_Bool image_data_is_opaque) OVERRIDE;
  int32_t MeasureText(const PP_TextRun_Dev* text) OVERRIDE;
  uint32_t CharacterOffsetForPixel(const PP_TextRun_Dev* text,
                                   int32_t pixel_position) OVERRIDE;
  int32_t PixelOffsetForCharacter(const PP_TextRun_Dev* text,
                                  uint32_t char_offset) OVERRIDE;

 private:
  scoped_ptr<pp::shared_impl::WebKitForwarding::Font> font_forwarding_;

  DISALLOW_COPY_AND_ASSIGN(PPB_Font_Impl);
};

}  // namespace ppapi
}  // namespace webkit

#endif  // WEBKIT_PLUGINS_PPAPI_PPB_FONT_IMPL_H_
