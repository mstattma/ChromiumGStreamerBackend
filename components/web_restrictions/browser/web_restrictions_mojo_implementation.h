// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_WEB_RESTRICTIONS_BROWSER_WEB_RESTRICTIONS_MOJO_IMPLEMENTATION_H_
#define COMPONENTS_WEB_RESTRICTIONS_BROWSER_WEB_RESTRICTIONS_MOJO_IMPLEMENTATION_H_

#include <string>

#include "base/macros.h"
#include "components/web_restrictions/interfaces/web_restrictions.mojom.h"
#include "mojo/public/cpp/bindings/strong_binding.h"

namespace web_restrictions {

class WebRestrictionsClient;

class WebRestrictionsMojoImplementation : public mojom::WebRestrictions {
 public:
  static void Create(WebRestrictionsClient* client,
                     mojo::InterfaceRequest<mojom::WebRestrictions> request);

 private:
  WebRestrictionsMojoImplementation(
      WebRestrictionsClient* client,
      mojo::InterfaceRequest<mojom::WebRestrictions> request);
  ~WebRestrictionsMojoImplementation() override;

  void GetResult(const std::string& url,
                 const GetResultCallback& callback) override;
  void RequestPermission(const std::string& url,
                         const RequestPermissionCallback& callback) override;

  mojo::StrongBinding<mojom::WebRestrictions> binding_;
  WebRestrictionsClient* web_restrictions_client_;
};

}  // namespace web_restrictions
#endif  // COMPONENTS_WEB_RESTRICTIONS_BROWSER_WEB_RESTRICTIONS_MOJO_IMPLEMENTATION_H_
