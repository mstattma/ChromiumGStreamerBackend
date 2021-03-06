// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/common/gpu_support_stub.h"

namespace ash {

GPUSupportStub::GPUSupportStub() {}

GPUSupportStub::~GPUSupportStub() {}

bool GPUSupportStub::IsPanelFittingDisabled() const {
  return false;
}

void GPUSupportStub::DisableGpuWatchdog() {}

void GPUSupportStub::GetGpuProcessHandles(
    const GetGpuProcessHandlesCallback& callback) const {}

}  // namespace ash
