// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_FILEAPI_FILE_SYSTEM_URL_REQUEST_JOB_H_
#define WEBKIT_FILEAPI_FILE_SYSTEM_URL_REQUEST_JOB_H_
#pragma once

#include <string>

#include "base/file_path.h"
#include "base/memory/scoped_callback_factory.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop_proxy.h"
#include "base/platform_file.h"
#include "base/task.h"
#include "net/base/completion_callback.h"
#include "net/http/http_byte_range.h"
#include "net/url_request/url_request_job.h"
#include "webkit/fileapi/file_system_url_request_job_base.h"

class GURL;

namespace net {
class FileStream;
}

namespace fileapi {
class FileSystemContext;

// A request job that handles reading filesystem: URLs
class FileSystemURLRequestJob : public FileSystemURLRequestJobBase {
 public:
  FileSystemURLRequestJob(
      net::URLRequest* request, FileSystemContext* file_system_context,
      scoped_refptr<base::MessageLoopProxy> file_thread_proxy);

  // URLRequestJob methods:
  virtual void Start();
  virtual void Kill();
  virtual bool ReadRawData(net::IOBuffer* buf, int buf_size, int* bytes_read);
  virtual bool IsRedirectResponse(GURL* location, int* http_status_code);
  virtual void SetExtraRequestHeaders(const net::HttpRequestHeaders& headers);
  virtual void GetResponseInfo(net::HttpResponseInfo* info);
  virtual int GetResponseCode() const;

  // FilterContext methods (via URLRequestJob):
  virtual bool GetMimeType(std::string* mime_type) const;

 protected:
  // FileSystemURLRequestJobBase methods.
  virtual void DidGetLocalPath(const FilePath& local_path);

 private:
  virtual ~FileSystemURLRequestJob();

  void DidResolve(base::PlatformFileError error_code,
                  const base::PlatformFileInfo& file_info);
  void DidOpen(base::PlatformFileError error_code,
               base::PassPlatformFile file, bool created);
  void DidRead(int result);

  ScopedRunnableMethodFactory<FileSystemURLRequestJob> method_factory_;
  base::ScopedCallbackFactory<FileSystemURLRequestJob> callback_factory_;
  net::CompletionCallbackImpl<FileSystemURLRequestJob> io_callback_;
  scoped_ptr<net::FileStream> stream_;
  bool is_directory_;
  scoped_ptr<net::HttpResponseInfo> response_info_;
  int64 remaining_bytes_;
  net::HttpByteRange byte_range_;

  DISALLOW_COPY_AND_ASSIGN(FileSystemURLRequestJob);
};

}  // namespace fileapi

#endif  // WEBKIT_FILEAPI_FILE_SYSTEM_URL_REQUEST_JOB_H_
