// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated. DO NOT EDIT!

// OpenGL ES interface.
#ifndef PPAPI_C_DEV_PPB_OPENGLES_DEV_H_
#define PPAPI_C_DEV_PPB_OPENGLES_DEV_H_

#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_stdint.h"

#ifndef __gl2_h_
typedef unsigned int GLenum;
typedef void GLvoid;
typedef long int GLintptr;
typedef int GLsizei;
typedef long int GLsizeiptr;
typedef int GLint;
typedef unsigned char GLboolean;
typedef unsigned int GLuint;
typedef unsigned int GLbitfield;
typedef short GLshort;
typedef float GLfloat;
typedef float GLclampf;
typedef int8_t GLbyte;
typedef uint8_t GLubyte;
typedef int32_t GLfixed;
typedef unsigned short GLushort;
typedef int GLclampx;
#endif  // __gl2_h_

// TODO(brettw) this string should be PPB_OpenGLES2;1.0 when this is frozen.
// The 2.0 is trying to describe the version of OpenGL ES which is different
// than the Pepper interface version.
#define PPB_OPENGLES2_DEV_INTERFACE_0_1 "PPB_OpenGLES(Dev);2.0"
#define PPB_OPENGLES2_DEV_INTERFACE PPB_OPENGLES2_DEV_INTERFACE_0_1

struct PPB_OpenGLES2_Dev {
  void (*ActiveTexture)(PP_Resource context, GLenum texture);
  void (*AttachShader)(PP_Resource context, GLuint program, GLuint shader);
  void (*BindAttribLocation)(
      PP_Resource context, GLuint program, GLuint index, const char* name);
  void (*BindBuffer)(PP_Resource context, GLenum target, GLuint buffer);
  void (*BindFramebuffer)(
      PP_Resource context, GLenum target, GLuint framebuffer);
  void (*BindRenderbuffer)(
      PP_Resource context, GLenum target, GLuint renderbuffer);
  void (*BindTexture)(PP_Resource context, GLenum target, GLuint texture);
  void (*BlendColor)(
      PP_Resource context, GLclampf red, GLclampf green, GLclampf blue,
      GLclampf alpha);
  void (*BlendEquation)(PP_Resource context, GLenum mode);
  void (*BlendEquationSeparate)(
      PP_Resource context, GLenum modeRGB, GLenum modeAlpha);
  void (*BlendFunc)(PP_Resource context, GLenum sfactor, GLenum dfactor);
  void (*BlendFuncSeparate)(
      PP_Resource context, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha,
      GLenum dstAlpha);
  void (*BufferData)(
      PP_Resource context, GLenum target, GLsizeiptr size, const void* data,
      GLenum usage);
  void (*BufferSubData)(
      PP_Resource context, GLenum target, GLintptr offset, GLsizeiptr size,
      const void* data);
  GLenum (*CheckFramebufferStatus)(PP_Resource context, GLenum target);
  void (*Clear)(PP_Resource context, GLbitfield mask);
  void (*ClearColor)(
      PP_Resource context, GLclampf red, GLclampf green, GLclampf blue,
      GLclampf alpha);
  void (*ClearDepthf)(PP_Resource context, GLclampf depth);
  void (*ClearStencil)(PP_Resource context, GLint s);
  void (*ColorMask)(
      PP_Resource context, GLboolean red, GLboolean green, GLboolean blue,
      GLboolean alpha);
  void (*CompileShader)(PP_Resource context, GLuint shader);
  void (*CompressedTexImage2D)(
      PP_Resource context, GLenum target, GLint level, GLenum internalformat,
      GLsizei width, GLsizei height, GLint border, GLsizei imageSize,
      const void* data);
  void (*CompressedTexSubImage2D)(
      PP_Resource context, GLenum target, GLint level, GLint xoffset,
      GLint yoffset, GLsizei width, GLsizei height, GLenum format,
      GLsizei imageSize, const void* data);
  void (*CopyTexImage2D)(
      PP_Resource context, GLenum target, GLint level, GLenum internalformat,
      GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
  void (*CopyTexSubImage2D)(
      PP_Resource context, GLenum target, GLint level, GLint xoffset,
      GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
  GLuint (*CreateProgram)(PP_Resource context);
  GLuint (*CreateShader)(PP_Resource context, GLenum type);
  void (*CullFace)(PP_Resource context, GLenum mode);
  void (*DeleteBuffers)(PP_Resource context, GLsizei n, const GLuint* buffers);
  void (*DeleteFramebuffers)(
      PP_Resource context, GLsizei n, const GLuint* framebuffers);
  void (*DeleteProgram)(PP_Resource context, GLuint program);
  void (*DeleteRenderbuffers)(
      PP_Resource context, GLsizei n, const GLuint* renderbuffers);
  void (*DeleteShader)(PP_Resource context, GLuint shader);
  void (*DeleteTextures)(
      PP_Resource context, GLsizei n, const GLuint* textures);
  void (*DepthFunc)(PP_Resource context, GLenum func);
  void (*DepthMask)(PP_Resource context, GLboolean flag);
  void (*DepthRangef)(PP_Resource context, GLclampf zNear, GLclampf zFar);
  void (*DetachShader)(PP_Resource context, GLuint program, GLuint shader);
  void (*Disable)(PP_Resource context, GLenum cap);
  void (*DisableVertexAttribArray)(PP_Resource context, GLuint index);
  void (*DrawArrays)(
      PP_Resource context, GLenum mode, GLint first, GLsizei count);
  void (*DrawElements)(
      PP_Resource context, GLenum mode, GLsizei count, GLenum type,
      const void* indices);
  void (*Enable)(PP_Resource context, GLenum cap);
  void (*EnableVertexAttribArray)(PP_Resource context, GLuint index);
  void (*Finish)(PP_Resource context);
  void (*Flush)(PP_Resource context);
  void (*FramebufferRenderbuffer)(
      PP_Resource context, GLenum target, GLenum attachment,
      GLenum renderbuffertarget, GLuint renderbuffer);
  void (*FramebufferTexture2D)(
      PP_Resource context, GLenum target, GLenum attachment, GLenum textarget,
      GLuint texture, GLint level);
  void (*FrontFace)(PP_Resource context, GLenum mode);
  void (*GenBuffers)(PP_Resource context, GLsizei n, GLuint* buffers);
  void (*GenerateMipmap)(PP_Resource context, GLenum target);
  void (*GenFramebuffers)(
      PP_Resource context, GLsizei n, GLuint* framebuffers);
  void (*GenRenderbuffers)(
      PP_Resource context, GLsizei n, GLuint* renderbuffers);
  void (*GenTextures)(PP_Resource context, GLsizei n, GLuint* textures);
  void (*GetActiveAttrib)(
      PP_Resource context, GLuint program, GLuint index, GLsizei bufsize,
      GLsizei* length, GLint* size, GLenum* type, char* name);
  void (*GetActiveUniform)(
      PP_Resource context, GLuint program, GLuint index, GLsizei bufsize,
      GLsizei* length, GLint* size, GLenum* type, char* name);
  void (*GetAttachedShaders)(
      PP_Resource context, GLuint program, GLsizei maxcount, GLsizei* count,
      GLuint* shaders);
  GLint (*GetAttribLocation)(
      PP_Resource context, GLuint program, const char* name);
  void (*GetBooleanv)(PP_Resource context, GLenum pname, GLboolean* params);
  void (*GetBufferParameteriv)(
      PP_Resource context, GLenum target, GLenum pname, GLint* params);
  GLenum (*GetError)(PP_Resource context);
  void (*GetFloatv)(PP_Resource context, GLenum pname, GLfloat* params);
  void (*GetFramebufferAttachmentParameteriv)(
      PP_Resource context, GLenum target, GLenum attachment, GLenum pname,
      GLint* params);
  void (*GetIntegerv)(PP_Resource context, GLenum pname, GLint* params);
  void (*GetProgramiv)(
      PP_Resource context, GLuint program, GLenum pname, GLint* params);
  void (*GetProgramInfoLog)(
      PP_Resource context, GLuint program, GLsizei bufsize, GLsizei* length,
      char* infolog);
  void (*GetRenderbufferParameteriv)(
      PP_Resource context, GLenum target, GLenum pname, GLint* params);
  void (*GetShaderiv)(
      PP_Resource context, GLuint shader, GLenum pname, GLint* params);
  void (*GetShaderInfoLog)(
      PP_Resource context, GLuint shader, GLsizei bufsize, GLsizei* length,
      char* infolog);
  void (*GetShaderPrecisionFormat)(
      PP_Resource context, GLenum shadertype, GLenum precisiontype,
      GLint* range, GLint* precision);
  void (*GetShaderSource)(
      PP_Resource context, GLuint shader, GLsizei bufsize, GLsizei* length,
      char* source);
  const GLubyte* (*GetString)(PP_Resource context, GLenum name);
  void (*GetTexParameterfv)(
      PP_Resource context, GLenum target, GLenum pname, GLfloat* params);
  void (*GetTexParameteriv)(
      PP_Resource context, GLenum target, GLenum pname, GLint* params);
  void (*GetUniformfv)(
      PP_Resource context, GLuint program, GLint location, GLfloat* params);
  void (*GetUniformiv)(
      PP_Resource context, GLuint program, GLint location, GLint* params);
  GLint (*GetUniformLocation)(
      PP_Resource context, GLuint program, const char* name);
  void (*GetVertexAttribfv)(
      PP_Resource context, GLuint index, GLenum pname, GLfloat* params);
  void (*GetVertexAttribiv)(
      PP_Resource context, GLuint index, GLenum pname, GLint* params);
  void (*GetVertexAttribPointerv)(
      PP_Resource context, GLuint index, GLenum pname, void** pointer);
  void (*Hint)(PP_Resource context, GLenum target, GLenum mode);
  GLboolean (*IsBuffer)(PP_Resource context, GLuint buffer);
  GLboolean (*IsEnabled)(PP_Resource context, GLenum cap);
  GLboolean (*IsFramebuffer)(PP_Resource context, GLuint framebuffer);
  GLboolean (*IsProgram)(PP_Resource context, GLuint program);
  GLboolean (*IsRenderbuffer)(PP_Resource context, GLuint renderbuffer);
  GLboolean (*IsShader)(PP_Resource context, GLuint shader);
  GLboolean (*IsTexture)(PP_Resource context, GLuint texture);
  void (*LineWidth)(PP_Resource context, GLfloat width);
  void (*LinkProgram)(PP_Resource context, GLuint program);
  void (*PixelStorei)(PP_Resource context, GLenum pname, GLint param);
  void (*PolygonOffset)(PP_Resource context, GLfloat factor, GLfloat units);
  void (*ReadPixels)(
      PP_Resource context, GLint x, GLint y, GLsizei width, GLsizei height,
      GLenum format, GLenum type, void* pixels);
  void (*ReleaseShaderCompiler)(PP_Resource context);
  void (*RenderbufferStorage)(
      PP_Resource context, GLenum target, GLenum internalformat, GLsizei width,
      GLsizei height);
  void (*SampleCoverage)(
      PP_Resource context, GLclampf value, GLboolean invert);
  void (*Scissor)(
      PP_Resource context, GLint x, GLint y, GLsizei width, GLsizei height);
  void (*ShaderBinary)(
      PP_Resource context, GLsizei n, const GLuint* shaders,
      GLenum binaryformat, const void* binary, GLsizei length);
  void (*ShaderSource)(
      PP_Resource context, GLuint shader, GLsizei count, const char** str,
      const GLint* length);
  void (*StencilFunc)(
      PP_Resource context, GLenum func, GLint ref, GLuint mask);
  void (*StencilFuncSeparate)(
      PP_Resource context, GLenum face, GLenum func, GLint ref, GLuint mask);
  void (*StencilMask)(PP_Resource context, GLuint mask);
  void (*StencilMaskSeparate)(PP_Resource context, GLenum face, GLuint mask);
  void (*StencilOp)(
      PP_Resource context, GLenum fail, GLenum zfail, GLenum zpass);
  void (*StencilOpSeparate)(
      PP_Resource context, GLenum face, GLenum fail, GLenum zfail,
      GLenum zpass);
  void (*TexImage2D)(
      PP_Resource context, GLenum target, GLint level, GLint internalformat,
      GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type,
      const void* pixels);
  void (*TexParameterf)(
      PP_Resource context, GLenum target, GLenum pname, GLfloat param);
  void (*TexParameterfv)(
      PP_Resource context, GLenum target, GLenum pname, const GLfloat* params);
  void (*TexParameteri)(
      PP_Resource context, GLenum target, GLenum pname, GLint param);
  void (*TexParameteriv)(
      PP_Resource context, GLenum target, GLenum pname, const GLint* params);
  void (*TexSubImage2D)(
      PP_Resource context, GLenum target, GLint level, GLint xoffset,
      GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type,
      const void* pixels);
  void (*Uniform1f)(PP_Resource context, GLint location, GLfloat x);
  void (*Uniform1fv)(
      PP_Resource context, GLint location, GLsizei count, const GLfloat* v);
  void (*Uniform1i)(PP_Resource context, GLint location, GLint x);
  void (*Uniform1iv)(
      PP_Resource context, GLint location, GLsizei count, const GLint* v);
  void (*Uniform2f)(PP_Resource context, GLint location, GLfloat x, GLfloat y);
  void (*Uniform2fv)(
      PP_Resource context, GLint location, GLsizei count, const GLfloat* v);
  void (*Uniform2i)(PP_Resource context, GLint location, GLint x, GLint y);
  void (*Uniform2iv)(
      PP_Resource context, GLint location, GLsizei count, const GLint* v);
  void (*Uniform3f)(
      PP_Resource context, GLint location, GLfloat x, GLfloat y, GLfloat z);
  void (*Uniform3fv)(
      PP_Resource context, GLint location, GLsizei count, const GLfloat* v);
  void (*Uniform3i)(
      PP_Resource context, GLint location, GLint x, GLint y, GLint z);
  void (*Uniform3iv)(
      PP_Resource context, GLint location, GLsizei count, const GLint* v);
  void (*Uniform4f)(
      PP_Resource context, GLint location, GLfloat x, GLfloat y, GLfloat z,
      GLfloat w);
  void (*Uniform4fv)(
      PP_Resource context, GLint location, GLsizei count, const GLfloat* v);
  void (*Uniform4i)(
      PP_Resource context, GLint location, GLint x, GLint y, GLint z, GLint w);
  void (*Uniform4iv)(
      PP_Resource context, GLint location, GLsizei count, const GLint* v);
  void (*UniformMatrix2fv)(
      PP_Resource context, GLint location, GLsizei count, GLboolean transpose,
      const GLfloat* value);
  void (*UniformMatrix3fv)(
      PP_Resource context, GLint location, GLsizei count, GLboolean transpose,
      const GLfloat* value);
  void (*UniformMatrix4fv)(
      PP_Resource context, GLint location, GLsizei count, GLboolean transpose,
      const GLfloat* value);
  void (*UseProgram)(PP_Resource context, GLuint program);
  void (*ValidateProgram)(PP_Resource context, GLuint program);
  void (*VertexAttrib1f)(PP_Resource context, GLuint indx, GLfloat x);
  void (*VertexAttrib1fv)(
      PP_Resource context, GLuint indx, const GLfloat* values);
  void (*VertexAttrib2f)(
      PP_Resource context, GLuint indx, GLfloat x, GLfloat y);
  void (*VertexAttrib2fv)(
      PP_Resource context, GLuint indx, const GLfloat* values);
  void (*VertexAttrib3f)(
      PP_Resource context, GLuint indx, GLfloat x, GLfloat y, GLfloat z);
  void (*VertexAttrib3fv)(
      PP_Resource context, GLuint indx, const GLfloat* values);
  void (*VertexAttrib4f)(
      PP_Resource context, GLuint indx, GLfloat x, GLfloat y, GLfloat z,
      GLfloat w);
  void (*VertexAttrib4fv)(
      PP_Resource context, GLuint indx, const GLfloat* values);
  void (*VertexAttribPointer)(
      PP_Resource context, GLuint indx, GLint size, GLenum type,
      GLboolean normalized, GLsizei stride, const void* ptr);
  void (*Viewport)(
      PP_Resource context, GLint x, GLint y, GLsizei width, GLsizei height);
};

#endif  // PPAPI_C_DEV_PPB_OPENGLES_DEV_H_

