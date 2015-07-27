// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_CONSTRAINED_WEB_DIALOG_UI_H_
#define CHROME_BROWSER_UI_WEBUI_CONSTRAINED_WEB_DIALOG_UI_H_

#include "base/compiler_specific.h"
#include "content/public/browser/web_ui_controller.h"
#include "ui/gfx/native_widget_types.h"

namespace gfx {
class Size;
}

namespace content {
class BrowserContext;
class RenderViewHost;
class WebContents;
}

namespace ui {
class WebDialogDelegate;
class WebDialogWebContentsDelegate;
}

class ConstrainedWebDialogDelegate {
 public:
  virtual const ui::WebDialogDelegate* GetWebDialogDelegate() const = 0;
  virtual ui::WebDialogDelegate* GetWebDialogDelegate() = 0;

  // Called when the dialog is being closed in response to a "dialogClose"
  // message from WebUI.
  virtual void OnDialogCloseFromWebUI() = 0;

  // If called, on dialog closure, the dialog will release its WebContents
  // instead of destroying it. After which point, the caller will own the
  // released WebContents.
  virtual void ReleaseWebContentsOnDialogClose() = 0;

  // Returns the WebContents owned by the constrained window.
  virtual content::WebContents* GetWebContents() = 0;

  // Returns the native type used to display the dialog.
  virtual gfx::NativeWindow GetNativeDialog() = 0;

  // Returns the minimum size for the dialog.
  virtual gfx::Size GetMinimumSize() const = 0;

  // Returns the maximum size for the dialog.
  virtual gfx::Size GetMaximumSize() const = 0;

  virtual gfx::Size GetPreferredSize() const = 0;

 protected:
  virtual ~ConstrainedWebDialogDelegate() {}
};

// ConstrainedWebDialogUI is a facility to show HTML WebUI content
// in a tab-modal constrained dialog.  It is implemented as an adapter
// between an WebDialogUI object and a web contents modal dialog.
//
// Since the web contents modal dialog requires platform-specific delegate
// implementations, this class is just a factory stub.
class ConstrainedWebDialogUI : public content::WebUIController {
 public:
  explicit ConstrainedWebDialogUI(content::WebUI* web_ui);
  ~ConstrainedWebDialogUI() override;

  // WebUIController implementation:
  void RenderViewCreated(content::RenderViewHost* render_view_host) override;

  // Sets the delegate on the WebContents.
  static void SetConstrainedDelegate(content::WebContents* web_contents,
                                     ConstrainedWebDialogDelegate* delegate);

 protected:
  // Returns the ConstrainedWebDialogDelegate saved with the WebContents.
  // Returns NULL if no such delegate is set.
  ConstrainedWebDialogDelegate* GetConstrainedDelegate();

 private:
  // JS Message Handler
  void OnDialogCloseMessage(const base::ListValue* args);

  DISALLOW_COPY_AND_ASSIGN(ConstrainedWebDialogUI);
};

// Create and show a constrained HTML dialog. The actual object that gets
// created is a ConstrainedWebDialogDelegate, which later triggers construction
// of a ConstrainedWebDialogUI object.
// |browser_context| is used to construct the constrained HTML dialog's
//                   WebContents.
// |delegate| controls the behavior of the dialog.
// |overshadowed| is the tab being overshadowed by the dialog.
ConstrainedWebDialogDelegate* ShowConstrainedWebDialog(
    content::BrowserContext* browser_context,
    ui::WebDialogDelegate* delegate,
    content::WebContents* overshadowed);

// Create and show a constrained HTML dialog with auto-resize enabled. The
// dialog is shown automatically after document load has completed to avoid UI
// jankiness.
// |browser_context| is used to construct the dialog's WebContents.
// |delegate| controls the behavior of the dialog.
// |overshadowed| is the tab being overshadowed by the dialog.
// |min_size| is the minimum size of the dialog.
// |max_size| is the maximum size of the dialog.
ConstrainedWebDialogDelegate* ShowConstrainedWebDialogWithAutoResize(
    content::BrowserContext* browser_context,
    ui::WebDialogDelegate* delegate,
    content::WebContents* overshadowed,
    const gfx::Size& min_size,
    const gfx::Size& max_size);

#endif  // CHROME_BROWSER_UI_WEBUI_CONSTRAINED_WEB_DIALOG_UI_H_
