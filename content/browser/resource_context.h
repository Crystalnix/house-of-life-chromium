// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RESOURCE_CONTEXT_H_
#define CONTENT_BROWSER_RESOURCE_CONTEXT_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

class ChromeAppCacheService;
class ChromeBlobStorageContext;
class ExtensionInfoMap;
class HostContentSettingsMap;
class HostZoomMap;
namespace fileapi {
class FileSystemContext;
}  // namespace fileapi
namespace net {
class HostResolver;
class URLRequestContext;
}  // namespace net
namespace prerender {
class PrerenderManager;
}  // namespace prerender
namespace webkit_database {
class DatabaseTracker;
}  // namespace webkit_database

namespace content {

// ResourceContext contains the relevant context information required for
// resource loading. It lives on the IO thread, although it is constructed on
// the UI thread. ResourceContext doesn't own anything it points to, it just
// holds pointers to relevant objects to resource loading.
class ResourceContext {
 public:
  virtual ~ResourceContext();

  net::HostResolver* host_resolver() const;
  void set_host_resolver(net::HostResolver* host_resolver);

  net::URLRequestContext* request_context() const;
  void set_request_context(net::URLRequestContext* request_context);

  ChromeAppCacheService* appcache_service() const;
  void set_appcache_service(ChromeAppCacheService* service);

  webkit_database::DatabaseTracker* database_tracker() const;
  void set_database_tracker(webkit_database::DatabaseTracker* tracker);

  fileapi::FileSystemContext* file_system_context() const;
  void set_file_system_context(fileapi::FileSystemContext* context);

  ChromeBlobStorageContext* blob_storage_context() const;
  void set_blob_storage_context(ChromeBlobStorageContext* context);

  HostZoomMap* host_zoom_map() const;
  void set_host_zoom_map(HostZoomMap* host_zoom_map);

  // =======================================================================
  // TODO(willchan): These don't belong in content/. Remove them eventually.

  // TODO(jam): Kill this one.
  HostContentSettingsMap* host_content_settings_map() const;
  void set_host_content_settings_map(
      HostContentSettingsMap* host_content_settings_map);

  // TODO(mpcomplete): Kill this one.
  const ExtensionInfoMap* extension_info_map() const;
  void set_extension_info_map(ExtensionInfoMap* extension_info_map);

  // TODO(cbentzel): Kill this one.
  const base::WeakPtr<prerender::PrerenderManager>& prerender_manager() const;
  void set_prerender_manager(
      const base::WeakPtr<prerender::PrerenderManager>& prerender_manager);

 protected:
  ResourceContext();

 private:
  virtual void EnsureInitialized() const = 0;

  net::HostResolver* host_resolver_;
  net::URLRequestContext* request_context_;
  ChromeAppCacheService* appcache_service_;
  webkit_database::DatabaseTracker* database_tracker_;
  fileapi::FileSystemContext* file_system_context_;
  ChromeBlobStorageContext* blob_storage_context_;
  HostZoomMap* host_zoom_map_;

  // =======================================================================
  // TODO(willchan): These don't belong in content/. Remove them eventually.

  HostContentSettingsMap* host_content_settings_map_;
  ExtensionInfoMap* extension_info_map_;
  base::WeakPtr<prerender::PrerenderManager> prerender_manager_;

  DISALLOW_COPY_AND_ASSIGN(ResourceContext);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RESOURCE_CONTEXT_H_
