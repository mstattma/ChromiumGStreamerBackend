// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/web/web_state/js/crw_js_window_id_manager.h"

#import "base/ios/weak_nsobject.h"
#import "base/mac/scoped_nsobject.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/sys_string_conversions.h"
#include "crypto/random.h"
#import "ios/web/web_state/js/page_script_util.h"

namespace {
// Number of random bytes in unique key for window ID. The length of the
// window ID will be twice this number, as it is hexadecimal encoded.
const size_t kUniqueKeyLength = 16;
}  // namespace

@interface CRWJSWindowIDManager () {
  // Web view used for script evaluation to inject window ID.
  base::scoped_nsobject<WKWebView> _webView;
  // Backs up property with the same name.
  base::scoped_nsobject<NSString> _windowID;
}

// Returns a string of randomized ASCII characters.
+ (NSString*)newUniqueKey;

@end

@implementation CRWJSWindowIDManager

- (NSString*)windowID {
  return _windowID;
}

- (instancetype)initWithWebView:(WKWebView*)webView {
  if ((self = [super init])) {
    _webView.reset([webView retain]);
    _windowID.reset([[self class] newUniqueKey]);
  }
  return self;
}

- (void)inject {
  _windowID.reset([[self class] newUniqueKey]);
  NSString* script = [web::GetPageScript(@"window_id")
      stringByReplacingOccurrencesOfString:@"$(WINDOW_ID)"
                                withString:_windowID];
  // WKUserScript may not be injected yet. Make windowID script return boolean
  // indicating whether the injection was successfull.
  NSString* scriptWithResult = [NSString
      stringWithFormat:@"if (!window.__gCrWeb) {false; } else { %@; true; }",
                       script];

  base::WeakNSObject<CRWJSWindowIDManager> weakSelf(self);
  [_webView evaluateJavaScript:scriptWithResult
             completionHandler:^(id result, NSError* error) {
               if (error) {
                 DCHECK(error.code == WKErrorWebViewInvalidated ||
                        error.code == WKErrorWebContentProcessTerminated);
                 return;
               }

               DCHECK_EQ(CFBooleanGetTypeID(), CFGetTypeID(result));
               if (![result boolValue]) {
                 // WKUserScript has not been injected yet. Retry window id
                 // injection, because it is critical for the system to
                 // function.
                 [weakSelf inject];
               }
             }];
}

#pragma mark - Private

+ (NSString*)newUniqueKey {
  char randomBytes[kUniqueKeyLength];
  crypto::RandBytes(randomBytes, kUniqueKeyLength);
  std::string result = base::HexEncode(randomBytes, kUniqueKeyLength);
  return [base::SysUTF8ToNSString(result) retain];
}

@end
