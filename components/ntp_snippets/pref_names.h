// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_NTP_SNIPPETS_PREF_NAMES_H_
#define COMPONENTS_NTP_SNIPPETS_PREF_NAMES_H_

namespace ntp_snippets {
namespace prefs {

extern const char kEnableSnippets[];

extern const char kSnippetHosts[];

// The pref name for today's count of NTPSnippetsFetcher requests, so far.
extern const char kSnippetFetcherRequestCount[];
// The pref name for today's count of NTPSnippetsFetcher interactive requests.
extern const char kSnippetFetcherInteractiveRequestCount[];
// The pref name for the current day for the counter of NTPSnippetsFetcher
// requests.
extern const char kSnippetFetcherRequestsDay[];

// The pref name for today's count of requests for article thumbnails, so far.
extern const char kSnippetThumbnailsRequestCount[];
// The pref name for today's count of interactive requests for article
// thumbnails, so far.
extern const char kSnippetThumbnailsInteractiveRequestCount[];
// The pref name for the current day for the counter of requests for article
// thumbnails.
extern const char kSnippetThumbnailsRequestsDay[];

extern const char kDismissedRecentOfflineTabSuggestions[];
extern const char kDismissedDownloadSuggestions[];

// The pref name for the time when M54 was first started on the device.
extern const char kBookmarksFirstM54Start[];

}  // namespace prefs
}  // namespace ntp_snippets

#endif  // COMPONENTS_NTP_SNIPPETS_PREF_NAMES_H_
