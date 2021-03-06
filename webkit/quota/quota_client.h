// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_QUOTA_QUOTA_CLIENT_H_
#define WEBKIT_QUOTA_QUOTA_CLIENT_H_

#include <set>
#include <list>

#include "base/callback.h"
#include "base/time.h"
#include "googleurl/src/gurl.h"
#include "webkit/quota/quota_types.h"

namespace quota {

// An abstract interface for quota manager clients.
// Each storage API must provide an implementation of this interface and
// register it to the quota manager.
// All the methods are assumed to be called on the IO thread in the browser.
class QuotaClient {
 public:
  typedef Callback1<int64>::Type GetUsageCallback;
  typedef Callback1<const std::set<GURL>&>::Type GetOriginsCallback;

  virtual ~QuotaClient() {}

  enum ID {
    kUnknown,
    kFileSystem,
    kDatabase,
    kAppcache,
    kIndexedDatabase,
    kMockStart,  // This needs to be the end of the enum.
  };

  virtual ID id() const = 0;

  // Called by the QuotaManager.
  // Gets the amount of data stored in the storage specified by
  // |origin_url| and |type|.
  virtual void GetOriginUsage(const GURL& origin_url,
                              StorageType type,
                              GetUsageCallback* callback) = 0;

  // Called by the QuotaManager.
  // Returns a list of origins that has data in the |type| storage.
  virtual void GetOriginsForType(StorageType type,
                                 GetOriginsCallback* callback) = 0;

  // Called by the QuotaManager.
  // Returns a list of origins that match the |host|.
  virtual void GetOriginsForHost(StorageType type,
                                 const std::string& host,
                                 GetOriginsCallback* callback) = 0;
};

typedef std::list<QuotaClient*> QuotaClientList;

}  // namespace quota

#endif  // WEBKIT_QUOTA_QUOTA_CLIENT_H_
