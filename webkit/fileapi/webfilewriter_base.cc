// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/fileapi/webfilewriter_base.h"

#include "base/logging.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileError.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFileWriterClient.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebURL.h"
#include "webkit/glue/webkit_glue.h"

namespace fileapi {

WebFileWriterBase::WebFileWriterBase(
     const GURL& path, WebKit::WebFileWriterClient* client)
  : path_(path),
    client_(client),
    operation_(kOperationNone),
    cancel_state_(kCancelNotInProgress) {
}

WebFileWriterBase::~WebFileWriterBase() {
}

void WebFileWriterBase::truncate(long long length) {
  DCHECK(kOperationNone == operation_);
  DCHECK(kCancelNotInProgress == cancel_state_);
  operation_ = kOperationTruncate;
  DoTruncate(path_, length);
}

void WebFileWriterBase::write(
      long long position,
      const WebKit::WebURL& blob_url) {
  DCHECK(kOperationNone == operation_);
  DCHECK(kCancelNotInProgress == cancel_state_);
  operation_ = kOperationWrite;
  DoWrite(path_, blob_url, position);
}

// When we cancel a write/truncate, we always get back the result of the write
// before the result of the cancel, no matter what happens.
// So we'll get back either
//   success [of the write/truncate, in a DidWrite(XXX, true)/DidSucceed() call]
//     followed by failure [of the cancel]; or
//   failure [of the write, either from cancel or other reasons] followed by
//     the result of the cancel.
// In the write case, there could also be queued up non-terminal DidWrite calls
// before any of that comes back, but there will always be a terminal write
// response [success or failure] after them, followed by the cancel result, so
// we can ignore non-terminal write responses, take the terminal write success
// or the first failure as the last write response, then know that the next
// thing to come back is the cancel response.  We only notify the
// AsyncFileWriterClient when it's all over.
void WebFileWriterBase::cancel() {
  DCHECK(kOperationWrite == operation_ || kOperationTruncate == operation_);
  if (kCancelNotInProgress != cancel_state_)
    return;
  cancel_state_ = kCancelSent;
  DoCancel();
}

void WebFileWriterBase::DidSucceed() {
  // Write never gets a DidSucceed call, so this is either a cancel or truncate
  // response.
  switch (cancel_state_) {
    case kCancelNotInProgress:
      // A truncate succeeded, with no complications.
      DCHECK(kOperationTruncate == operation_);
      operation_ = kOperationNone;
      client_->didTruncate();
      break;
    case kCancelSent:
      DCHECK(kOperationTruncate == operation_);
      // This is the success call of the truncate, which we'll eat, even though
      // it succeeded before the cancel got there.  We accepted the cancel call,
      // so the truncate will eventually return an error.
      cancel_state_ = kCancelReceivedWriteResponse;
      break;
    case kCancelReceivedWriteResponse:
      // This is the success of the cancel operation.
      FinishCancel();
      break;
    default:
      NOTREACHED();
  }
}

void WebFileWriterBase::DidFail(base::PlatformFileError error_code) {
  DCHECK(kOperationNone != operation_);
  switch (cancel_state_) {
    case kCancelNotInProgress:
      // A write or truncate failed.
      operation_ = kOperationNone;
      client_->didFail(
          webkit_glue::PlatformFileErrorToWebFileError(error_code));
      break;
    case kCancelSent:
      // This is the failure of a write or truncate; the next message should be
      // the result of the cancel.  We don't assume that it'll be a success, as
      // the write/truncate could have failed for other reasons.
      cancel_state_ = kCancelReceivedWriteResponse;
      break;
    case kCancelReceivedWriteResponse:
      // The cancel reported failure, meaning that the write or truncate
      // finished before the cancel got there.  But we suppressed the
      // write/truncate's response, and will now report that it was cancelled.
      FinishCancel();
      break;
    default:
      NOTREACHED();
  }
}

void WebFileWriterBase::DidWrite(int64 bytes, bool complete) {
  DCHECK(kOperationWrite == operation_);
  switch (cancel_state_) {
    case kCancelNotInProgress:
      if (complete)
        operation_ = kOperationNone;
      client_->didWrite(bytes, complete);
      break;
    case kCancelSent:
      // This is the success call of the write, which we'll eat, even though
      // it succeeded before the cancel got there.  We accepted the cancel call,
      // so the write will eventually return an error.
      if (complete)
        cancel_state_ = kCancelReceivedWriteResponse;
      break;
    case kCancelReceivedWriteResponse:
    default:
      NOTREACHED();
  }
}

void WebFileWriterBase::FinishCancel() {
  DCHECK(kCancelReceivedWriteResponse == cancel_state_);
  DCHECK(kOperationNone != operation_);
  cancel_state_ = kCancelNotInProgress;
  operation_ = kOperationNone;
  client_->didFail(WebKit::WebFileErrorAbort);
}

}  // namespace fileapi
