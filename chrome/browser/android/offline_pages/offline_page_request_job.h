// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_ANDROID_OFFLINE_PAGES_OFFLINE_PAGE_REQUEST_JOB_H_
#define CHROME_BROWSER_ANDROID_OFFLINE_PAGES_OFFLINE_PAGE_REQUEST_JOB_H_

#include "base/memory/weak_ptr.h"
#include "content/public/browser/resource_request_info.h"
#include "content/public/common/resource_type.h"
#include "net/url_request/url_request_file_job.h"

namespace base {
class FilePath;
}

namespace offline_pages {

// Header that indicates that the offline page should be loaded if it exists
// regardless current network conditions. Its value is a comma/space separated
// name-value pair that may provide reason or define custom behavior.
extern const char kLoadingOfflinePageHeader[];
// The name used in name-value pair of kLoadingOfflinePageHeader to denote the
// reason for loading offline page.
extern const char kLoadingOfflinePageReason[];
// Possible values in name-value pair that denote the reason for loading offline
// page.
extern const char kLoadingOfflinePageDueToNetError[];

// A request job that serves content from offline file.
class OfflinePageRequestJob : public net::URLRequestFileJob {
 public:
  // This enum is used for UMA reporting. It contains all possible outcomes of
  // handling requests that might service offline page in different network
  // conditions. Generally one of these outcomes will happen.
  // The fringe errors (like no OfflinePageModel, etc.) are not reported due
  // to their low probability.
  // NOTE: because this is used for UMA reporting, these values should not be
  // changed or reused; new values should be ended immediately before the MAX
  // value. Make sure to update the histogram enum
  // (OfflinePagesAggregatedRequestResult in histograms.xml) accordingly.
  // Public for testing.
  enum class AggregatedRequestResult {
    SHOW_OFFLINE_ON_DISCONNECTED_NETWORK,
    PAGE_NOT_FOUND_ON_DISCONNECTED_NETWORK,
    SHOW_OFFLINE_ON_FLAKY_NETWORK,
    PAGE_NOT_FOUND_ON_FLAKY_NETWORK,
    SHOW_OFFLINE_ON_PROHIBITIVELY_SLOW_NETWORK,
    PAGE_NOT_FOUND_ON_PROHIBITIVELY_SLOW_NETWORK,
    PAGE_NOT_FRESH_ON_PROHIBITIVELY_SLOW_NETWORK,
    SHOW_OFFLINE_ON_CONNECTED_NETWORK,
    PAGE_NOT_FOUND_ON_CONNECTED_NETWORK,
    NO_TAB_ID,
    NO_WEB_CONTENTS,
    SHOW_NET_ERROR_PAGE,
    AGGREGATED_REQUEST_RESULT_MAX
  };

  // Delegate that allows tests to overwrite certain behaviors.
  class Delegate {
   public:
    using TabIdGetter = base::Callback<bool(content::WebContents*, int*)>;

    virtual ~Delegate() {}

    virtual content::ResourceRequestInfo::WebContentsGetter
    GetWebContentsGetter(net::URLRequest* request) const = 0;

    virtual TabIdGetter GetTabIdGetter() const = 0;
  };

  // Reports the aggregated result combining both request result and network
  // state.
  static void ReportAggregatedRequestResult(AggregatedRequestResult result);

  // Creates and returns a job to serve the offline page. Nullptr is returned if
  // offline page cannot or should not be served.
  static OfflinePageRequestJob* Create(void* profile_id,
                                       net::URLRequest* request,
                                       net::NetworkDelegate* network_delegate);

  ~OfflinePageRequestJob() override;

  // net::URLRequestJob overrides:
  void Start() override;
  void Kill() override;

  void OnOfflineFilePathAvailable(const base::FilePath& offline_file_path);

  void SetDelegateForTesting(std::unique_ptr<Delegate> delegate);

 private:
  OfflinePageRequestJob(void* profile_id,
                        net::URLRequest* request,
                        net::NetworkDelegate* network_delegate);

  void StartAsync();

  // Restarts the request job in order to fall back to the default handling.
  void FallbackToDefault();

  // The profile for processing offline pages.
  void* profile_id_;

  std::unique_ptr<Delegate> delegate_;

  base::WeakPtrFactory<OfflinePageRequestJob> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(OfflinePageRequestJob);
};

}  // namespace offline_pages

#endif  // CHROME_BROWSER_ANDROID_OFFLINE_PAGES_OFFLINE_PAGE_REQUEST_JOB_H_
