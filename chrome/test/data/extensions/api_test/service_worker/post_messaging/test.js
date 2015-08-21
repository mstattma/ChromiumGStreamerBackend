// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

chrome.tabs.create({url: chrome.extension.getURL("page.html")});
var message = "";

navigator.serviceWorker.addEventListener('message', function(event) {
    message = event.data;
});
