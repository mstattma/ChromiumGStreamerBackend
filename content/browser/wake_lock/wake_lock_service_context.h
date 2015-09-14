// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_WAKE_LOCK_WAKE_LOCK_SERVICE_CONTEXT_H_
#define CONTENT_BROWSER_WAKE_LOCK_WAKE_LOCK_SERVICE_CONTEXT_H_

#include <set>

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "content/browser/wake_lock/wake_lock_service_impl.h"
#include "content/common/content_export.h"
#include "content/public/browser/web_contents_observer.h"
#include "mojo/public/cpp/bindings/interface_request.h"

namespace content {

class PowerSaveBlocker;
class RenderFrameHost;
class WebContents;

class CONTENT_EXPORT WakeLockServiceContext: public WebContentsObserver {
 public:
  explicit WakeLockServiceContext(WebContents* web_contents);
  ~WakeLockServiceContext() override;

  // Creates a WakeLockServiceImpl that is strongly bound to |request|.
  void CreateService(int frame_id,
                     mojo::InterfaceRequest<WakeLockService> request);

  // WebContentsObserver implementation.
  void RenderFrameDeleted(RenderFrameHost* render_frame_host) override;

  // Requests wake lock for |frame_id|.
  void RequestWakeLock(int frame_id);

  // Cancels wake lock request for |frame_id|.
  void CancelWakeLock(int frame_id);

  // Used by tests.
  bool HasWakeLockForTests() const;

 private:
  void CreateWakeLock();
  void RemoveWakeLock();
  void UpdateWakeLock();

  // Set of all FrameTreeNode ids currently requesting wake lock.
  std::set<int> frames_requesting_lock_;

  // The actual power save blocker for screen.
  scoped_ptr<PowerSaveBlocker> wake_lock_;

  base::WeakPtrFactory<WakeLockServiceContext> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WakeLockServiceContext);
};

}  // namespace content

#endif  // CONTENT_BROWSER_WAKE_LOCK_WAKE_LOCK_SERVICE_CONTEXT_H_
