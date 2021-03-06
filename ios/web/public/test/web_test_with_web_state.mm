// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/web/public/test/web_test_with_web_state.h"

#import <WebKit/WebKit.h>

#include "base/base64.h"
#include "base/strings/sys_string_conversions.h"
#include "base/test/ios/wait_util.h"
#import "ios/testing/ocmock_complex_type_helper.h"
#import "ios/web/navigation/crw_session_controller.h"
#import "ios/web/public/web_state/url_verification_constants.h"
#import "ios/web/web_state/ui/crw_web_controller.h"
#import "ios/web/web_state/web_state_impl.h"

// Helper Mock to stub out API with C++ objects in arguments.
@interface WebDelegateMock : OCMockComplexTypeHelper
@end

@implementation WebDelegateMock
// Stub implementation always returns YES.
- (BOOL)webController:(CRWWebController*)webController
        shouldOpenURL:(const GURL&)url
      mainDocumentURL:(const GURL&)mainDocumentURL
          linkClicked:(BOOL)linkClicked {
  return YES;
}
@end

namespace {
// Returns CRWWebController for the given |web_state|.
CRWWebController* GetWebController(web::WebState* web_state) {
  web::WebStateImpl* web_state_impl =
      static_cast<web::WebStateImpl*>(web_state);
  return web_state_impl->GetWebController();
}
}  // namespace

namespace web {

WebTestWithWebState::WebTestWithWebState() {}

WebTestWithWebState::~WebTestWithWebState() {}

static int s_html_load_count;

void WebTestWithWebState::SetUp() {
  WebTest::SetUp();
  std::unique_ptr<WebStateImpl> web_state(new WebStateImpl(GetBrowserState()));
  web_state->GetNavigationManagerImpl().InitializeSession(nil, nil, NO, 0);
  web_state->SetWebUsageEnabled(true);
  web_state_.reset(web_state.release());

  // Force generation of child views; necessary for some tests.
  [GetWebController(web_state_.get()) triggerPendingLoad];
  s_html_load_count = 0;
}

void WebTestWithWebState::TearDown() {
  web_state_.reset();
  WebTest::TearDown();
}

void WebTestWithWebState::WillProcessTask(
    const base::PendingTask& pending_task) {
  // Nothing to do.
}

void WebTestWithWebState::DidProcessTask(
    const base::PendingTask& pending_task) {
  processed_a_task_ = true;
}

void WebTestWithWebState::LoadHtml(NSString* html, const GURL& url) {
  ASSERT_FALSE(web_state()->IsLoading());

  CRWWebController* web_controller = GetWebController(web_state());
  [web_controller loadHTML:html forURL:url];

  // Wait until the navigation is committed to update MIME type.
  ASSERT_EQ(LOAD_REQUESTED, web_controller.loadPhase);
  base::TimeDelta spin_delay = base::TimeDelta::FromMilliseconds(10);
  while (web_controller.loadPhase != PAGE_LOADING)
    base::test::ios::SpinRunLoopWithMaxDelay(spin_delay);

  // loadHTML:forURL: does not notify web view delegate about received response,
  // so web controller does not get a chance to properly update MIME type and it
  // should be set manually after navigation is committed but before WebState
  // signal load completion and clients will start checking if MIME type is in
  // fact HTML.
  [web_controller webStateImpl]->SetContentsMimeType("text/html");

  // Wait until the page is loaded.
  ASSERT_EQ(PAGE_LOADING, web_controller.loadPhase);
  while (web_controller.loadPhase != PAGE_LOADED)
    base::test::ios::SpinRunLoopWithMaxDelay(spin_delay);

  // Wait until scripts execution becomes possible.
  base::test::ios::WaitUntilCondition(^bool {
    return [ExecuteJavaScript(@"0;") isEqual:@0];
  });
}

void WebTestWithWebState::LoadHtml(NSString* html) {
  GURL url("https://chromium.test/");
  LoadHtml(html, url);
}

void WebTestWithWebState::LoadHtml(const std::string& html) {
  LoadHtml(base::SysUTF8ToNSString(html));
}

void WebTestWithWebState::WaitForBackgroundTasks() {
  // Because tasks can add new tasks to either queue, the loop continues until
  // the first pass where no activity is seen from either queue.
  bool activitySeen = false;
  base::MessageLoop* messageLoop = base::MessageLoop::current();
  messageLoop->AddTaskObserver(this);
  do {
    activitySeen = false;

    // Yield to the iOS message queue, e.g. [NSObject performSelector:] events.
    if (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) ==
        kCFRunLoopRunHandledSource)
      activitySeen = true;

    // Yield to the Chromium message queue, e.g. WebThread::PostTask()
    // events.
    processed_a_task_ = false;
    messageLoop->RunUntilIdle();
    if (processed_a_task_)  // Set in TaskObserver method.
      activitySeen = true;

  } while (activitySeen);
  messageLoop->RemoveTaskObserver(this);
}

void WebTestWithWebState::WaitForCondition(ConditionBlock condition) {
  base::MessageLoop* messageLoop = base::MessageLoop::current();
  DCHECK(messageLoop);
  base::test::ios::WaitUntilCondition(condition, messageLoop,
                                      base::TimeDelta::FromSeconds(10));
}

id WebTestWithWebState::ExecuteJavaScript(NSString* script) {
  __block base::scoped_nsprotocol<id> executionResult;
  __block bool executionCompleted = false;
  [GetWebController(web_state())
      executeJavaScript:script
      completionHandler:^(id result, NSError* error) {
        executionResult.reset([result copy]);
        executionCompleted = true;
      }];
  base::test::ios::WaitUntilCondition(^{
    return executionCompleted;
  });
  return [[executionResult retain] autorelease];
}

std::string WebTestWithWebState::BaseUrl() const {
  web::URLVerificationTrustLevel unused_level;
  return web_state()->GetCurrentURL(&unused_level).spec();
}

web::WebState* WebTestWithWebState::web_state() {
  return web_state_.get();
}

const web::WebState* WebTestWithWebState::web_state() const {
  return web_state_.get();
}

NSString* WebTestWithWebState::CreateLoadCheck() {
  return [NSString stringWithFormat:@"<p style=\"display: none;\">%d</p>",
                                    s_html_load_count++];
}

}  // namespace web
