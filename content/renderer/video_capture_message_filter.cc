// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/video_capture_message_filter.h"

#include "content/common/child_process.h"
#include "content/common/video_capture_messages.h"
#include "content/renderer/render_thread.h"

VideoCaptureMessageFilter::VideoCaptureMessageFilter(int32 route_id)
    : last_device_id_(0),
      route_id_(route_id),
      channel_(NULL) {
}

VideoCaptureMessageFilter::~VideoCaptureMessageFilter() {
}

bool VideoCaptureMessageFilter::Send(IPC::Message* message) {
  if (!channel_) {
    delete message;
    return false;
  }

  if (!message_loop_proxy_->BelongsToCurrentThread()) {
    // Can only access the IPC::Channel on the IPC thread since it's not thread
    // safe.
    message_loop_proxy_->PostTask(FROM_HERE,
        NewRunnableMethod(this, &VideoCaptureMessageFilter::Send, message));
    return true;
  }

  message->set_routing_id(route_id_);
  return channel_->Send(message);
}

bool VideoCaptureMessageFilter::OnMessageReceived(const IPC::Message& message) {
  if (message.routing_id() != route_id_)
    return false;

  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(VideoCaptureMessageFilter, message)
    IPC_MESSAGE_HANDLER(VideoCaptureMsg_BufferReady, OnBufferReceived)
    IPC_MESSAGE_HANDLER(VideoCaptureMsg_StateChanged, OnDeviceStateChanged)
    IPC_MESSAGE_HANDLER(VideoCaptureMsg_DeviceInfo, OnDeviceInfoReceived)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void VideoCaptureMessageFilter::OnFilterAdded(IPC::Channel* channel) {
  VLOG(1) << "VideoCaptureMessageFilter::OnFilterAdded()";
  // Captures the message loop proxy for IPC.
  message_loop_proxy_ = base::MessageLoopProxy::CreateForCurrentThread();
  channel_ = channel;
}

void VideoCaptureMessageFilter::OnFilterRemoved() {
  channel_ = NULL;
}

void VideoCaptureMessageFilter::OnChannelClosing() {
  channel_ = NULL;
}

void VideoCaptureMessageFilter::OnBufferReceived(const IPC::Message& msg,
                                                 int device_id,
                                                 TransportDIB::Handle handle,
                                                 base::Time timestamp) {
  Delegate* delegate = NULL;
  if (delegates_.find(device_id) != delegates_.end())
    delegate = delegates_.find(device_id)->second;

  if (!delegate) {
    DLOG(WARNING) << "OnBufferReceived: Got video frame buffer for a "
        "non-existent or removed video capture.";

    // Send the buffer back to Host in case it's waiting for all buffers
    // to be returned.
    Send(new VideoCaptureHostMsg_BufferReady(0, device_id, handle));
    return;
  }

  delegate->OnBufferReceived(handle, timestamp);
}

void VideoCaptureMessageFilter::OnDeviceStateChanged(
    int device_id,
    const media::VideoCapture::State& state) {
  Delegate* delegate = NULL;
  if (delegates_.find(device_id) != delegates_.end())
    delegate = delegates_.find(device_id)->second;
  if (!delegate) {
    DLOG(WARNING) << "OnDeviceStateChanged: Got video capture event for a "
        "non-existent or removed video capture.";
    return;
  }
  delegate->OnStateChanged(state);
}

void VideoCaptureMessageFilter::OnDeviceInfoReceived(
    const IPC::Message& msg,
    int device_id,
    media::VideoCaptureParams& params) {
  Delegate* delegate = NULL;
  if (delegates_.find(device_id) != delegates_.end())
    delegate = delegates_.find(device_id)->second;
  if (!delegate) {
    DLOG(WARNING) << "OnDeviceInfoReceived: Got video capture event for a "
        "non-existent or removed video capture.";
    return;
  }
  delegate->OnDeviceInfoReceived(params);
}

int32 VideoCaptureMessageFilter::AddDelegate(Delegate* delegate) {
  if (++last_device_id_ <= 0)
    last_device_id_ = 1;
  while (delegates_.find(last_device_id_) != delegates_.end())
    last_device_id_++;

  delegates_[last_device_id_] = delegate;
  return last_device_id_;
}

void VideoCaptureMessageFilter::RemoveDelegate(Delegate* delegate) {
  for (Delegates::iterator it = delegates_.begin();
       it != delegates_.end(); it++) {
    if (it->second == delegate) {
      delegates_.erase(it);
      break;
    }
  }
}

void VideoCaptureMessageFilter::AddFilter() {
  if (MessageLoop::current() != ChildThread::current()->message_loop()) {
    ChildThread::current()->message_loop()->PostTask(
      FROM_HERE, NewRunnableMethod(this,
                                   &VideoCaptureMessageFilter::AddFilter));
    return;
  }

  RenderThread::current()->AddFilter(this);
}
