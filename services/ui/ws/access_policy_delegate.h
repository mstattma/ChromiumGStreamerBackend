// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_UI_WS_ACCESS_POLICY_DELEGATE_H_
#define SERVICES_UI_WS_ACCESS_POLICY_DELEGATE_H_

#include <vector>

#include "base/containers/hash_tables.h"
#include "services/ui/ws/ids.h"

namespace ui {

namespace ws {

class ServerWindow;

// Delegate used by the AccessPolicy implementations to get state.
class AccessPolicyDelegate {
 public:
  // Returns true if the tree has |window| as one of its roots.
  virtual bool HasRootForAccessPolicy(const ServerWindow* window) const = 0;

  // Returns true if |window| has been exposed to the client.
  virtual bool IsWindowKnownForAccessPolicy(
      const ServerWindow* window) const = 0;

  // Returns true if Embed(window) has been invoked on |window|.
  virtual bool IsWindowRootOfAnotherTreeForAccessPolicy(
      const ServerWindow* window) const = 0;

 protected:
  virtual ~AccessPolicyDelegate() {}
};

}  // namespace ws

}  // namespace ui

#endif  // SERVICES_UI_WS_ACCESS_POLICY_DELEGATE_H_
