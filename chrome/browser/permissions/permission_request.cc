// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/permissions/permission_request.h"

#include "ui/gfx/vector_icons_public.h"

gfx::VectorIconId PermissionRequest::GetVectorIconId() const {
  return gfx::VectorIconId::VECTOR_ICON_NONE;
}

PermissionRequestType PermissionRequest::GetPermissionRequestType() const {
  return PermissionRequestType::UNKNOWN;
}

PermissionRequestGestureType PermissionRequest::GetGestureType() const {
  return PermissionRequestGestureType::UNKNOWN;
}
