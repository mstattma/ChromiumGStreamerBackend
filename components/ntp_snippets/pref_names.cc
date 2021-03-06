// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/ntp_snippets/pref_names.h"

namespace ntp_snippets {
namespace prefs {

const char kEnableSnippets[] = "ntp_snippets.enable";

const char kSnippetHosts[] = "ntp_snippets.hosts";

const char kSnippetFetcherRequestCount[] =
    "ntp.request_throttler.suggestion_fetcher.count";
const char kSnippetFetcherInteractiveRequestCount[] =
    "ntp.request_throttler.suggestion_fetcher.interactive_count";
const char kSnippetFetcherRequestsDay[] =
    "ntp.request_throttler.suggestion_fetcher.day";

const char kSnippetThumbnailsRequestCount[] =
    "ntp.request_throttler.suggestion_thumbnails.count";
const char kSnippetThumbnailsInteractiveRequestCount[] =
    "ntp.request_throttler.suggestion_thumbnails.interactive_count";
const char kSnippetThumbnailsRequestsDay[] =
    "ntp.request_throttler.suggestion_thumbnails.day";

const char kDismissedRecentOfflineTabSuggestions[] =
    "ntp_suggestions.offline_pages.recent_tabs.dismissed_ids";
const char kDismissedDownloadSuggestions[] =
    "ntp_suggestions.offline_pages.downloads.dismissed_ids";

const char kBookmarksFirstM54Start[] =
    "ntp_suggestions.bookmarks.first_M54_start";

}  // namespace prefs
}  // namespace ntp_snippets
