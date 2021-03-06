// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ASH_LAUNCHER_CHROME_MASH_SHELF_CONTROLLER_H_
#define CHROME_BROWSER_UI_ASH_LAUNCHER_CHROME_MASH_SHELF_CONTROLLER_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ash/public/interfaces/shelf.mojom.h"
#include "chrome/browser/ui/app_icon_loader.h"
#include "chrome/browser/ui/ash/launcher/launcher_controller_helper.h"
#include "mojo/public/cpp/bindings/associated_binding.h"

class ChromeShelfItemDelegate;

// ChromeMashShelfController manages chrome's interaction with the mash shelf.
class ChromeMashShelfController : public ash::mojom::ShelfObserver,
                                  public AppIconLoaderDelegate {
 public:
  ChromeMashShelfController();
  ~ChromeMashShelfController() override;

  void LaunchItem(const std::string& app_id);

 private:
  void Init();

  void PinAppsFromPrefs();

  AppIconLoader* GetAppIconLoaderForApp(const std::string& app_id);

  // ash::mojom::ShelfObserver:
  void OnShelfCreated(int64_t display_id) override;
  void OnAlignmentChanged(ash::ShelfAlignment alignment,
                          int64_t display_id) override;
  void OnAutoHideBehaviorChanged(ash::ShelfAutoHideBehavior auto_hide,
                                 int64_t display_id) override;

  // AppIconLoaderDelegate:
  void OnAppImageUpdated(const std::string& app_id,
                         const gfx::ImageSkia& image) override;

  LauncherControllerHelper helper_;
  ash::mojom::ShelfControllerPtr shelf_controller_;
  mojo::AssociatedBinding<ash::mojom::ShelfObserver> observer_binding_;
  std::map<std::string, std::unique_ptr<ChromeShelfItemDelegate>>
      app_id_to_item_delegate_;

  // Used to load the images for app items.
  std::vector<std::unique_ptr<AppIconLoader>> app_icon_loaders_;

  DISALLOW_COPY_AND_ASSIGN(ChromeMashShelfController);
};

#endif  // CHROME_BROWSER_UI_ASH_LAUNCHER_CHROME_MASH_SHELF_CONTROLLER_H_
