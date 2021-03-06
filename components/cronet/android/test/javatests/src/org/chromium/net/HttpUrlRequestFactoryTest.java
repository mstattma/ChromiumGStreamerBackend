// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;

import org.chromium.base.test.util.Feature;
import org.chromium.net.CronetTestBase.OnlyRunNativeCronet;
import org.chromium.net.test.EmbeddedTestServer;

import java.io.File;
import java.util.HashMap;
import java.util.regex.Pattern;

/**
 * Tests for {@link HttpUrlRequestFactory}
 */
@SuppressWarnings("deprecation")
public class HttpUrlRequestFactoryTest extends CronetTestBase {
    private EmbeddedTestServer mTestServer;
    private String mUrl;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mTestServer = EmbeddedTestServer.createAndStartDefaultServer(getContext());
        mUrl = mTestServer.getURL("/echo?status=200");
    }

    @Override
    protected void tearDown() throws Exception {
        mTestServer.stopAndDestroyServer();
        super.tearDown();
    }

    @SmallTest
    @Feature({"Cronet"})
    public void testCreateFactory() throws Throwable {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        builder.enableQuic(true);
        builder.addQuicHint("www.google.com", 443, 443);
        builder.addQuicHint("www.youtube.com", 443, 443);
        builder.setLibraryName("cronet_tests");
        HttpUrlRequestFactory factory = HttpUrlRequestFactory.createFactory(getContext(), builder);
        assertNotNull("Factory should be created", factory);
        assertTrue("Factory should be Chromium/n.n.n.n@r but is "
                           + factory.getName(),
                   Pattern.matches("Chromium/\\d+\\.\\d+\\.\\d+\\.\\d+@\\w+",
                           factory.getName()));
    }

    @SmallTest
    @Feature({"Cronet"})
    @OnlyRunNativeCronet
    public void testCreateLegacyFactory() {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        builder.enableLegacyMode(true);

        HttpUrlRequestFactory factory = HttpUrlRequestFactory.createFactory(getContext(), builder);
        assertNotNull("Factory should be created", factory);
        assertTrue("Factory should be HttpUrlConnection/n.n.n.n@r but is "
                           + factory.getName(),
                   Pattern.matches(
                           "HttpUrlConnection/\\d+\\.\\d+\\.\\d+\\.\\d+@\\w+",
                           factory.getName()));
        HashMap<String, String> headers = new HashMap<String, String>();
        TestHttpUrlRequestListener listener = new TestHttpUrlRequestListener();
        HttpUrlRequest request = factory.createRequest(mUrl, 0, headers, listener);
        request.start();
        listener.blockForComplete();
        assertEquals(200, listener.mHttpStatusCode);
        assertEquals("OK", listener.mHttpStatusText);
    }

    @SmallTest
    @Feature({"Cronet"})
    @OnlyRunNativeCronet
    public void testCreateLegacyFactoryUsingUrlRequestContextConfig() {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        builder.enableLegacyMode(true);

        HttpUrlRequestFactory factory = HttpUrlRequestFactory.createFactory(getContext(), builder);
        assertNotNull("Factory should be created", factory);
        assertTrue("Factory should be HttpUrlConnection/n.n.n.n@r but is "
                           + factory.getName(),
                   Pattern.matches(
                           "HttpUrlConnection/\\d+\\.\\d+\\.\\d+\\.\\d+@\\w+",
                           factory.getName()));
        HashMap<String, String> headers = new HashMap<String, String>();
        TestHttpUrlRequestListener listener = new TestHttpUrlRequestListener();
        HttpUrlRequest request = factory.createRequest(mUrl, 0, headers, listener);
        request.start();
        listener.blockForComplete();
        assertEquals(200, listener.mHttpStatusCode);
        assertEquals("OK", listener.mHttpStatusText);
    }

    @SmallTest
    @Feature({"Cronet"})
    public void testQuicHintHost() {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        builder.addQuicHint("www.google.com", 443, 443);
        try {
            builder.addQuicHint("https://www.google.com", 443, 443);
        } catch (IllegalArgumentException e) {
            return;
        }
        fail("IllegalArgumentException must be thrown");
    }

    @SmallTest
    @Feature({"Cronet"})
    public void testConfigUserAgent() throws Throwable {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        String userAgentName = "User-Agent";
        String userAgentValue = "User-Agent-Value";
        builder.setUserAgent(userAgentValue);
        builder.setLibraryName("cronet_tests");
        HttpUrlRequestFactory factory = HttpUrlRequestFactory.createFactory(getContext(), builder);
        assertTrue(NativeTestServer.startNativeTestServer(getContext()));
        String url = NativeTestServer.getEchoHeaderURL(userAgentName);
        TestHttpUrlRequestListener listener = new TestHttpUrlRequestListener();
        HashMap<String, String> headers = new HashMap<String, String>();
        HttpUrlRequest request = factory.createRequest(
                url, 0, headers, listener);
        request.start();
        listener.blockForComplete();
        assertEquals(userAgentValue, listener.mResponseAsString);
        NativeTestServer.shutdownNativeTestServer();
    }

    @SmallTest
    @Feature({"Cronet"})
    public void testConfigUserAgentLegacy() throws Throwable {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        String userAgentName = "User-Agent";
        String userAgentValue = "User-Agent-Value";
        builder.setUserAgent(userAgentValue);
        builder.enableLegacyMode(true);
        HttpUrlRequestFactory factory = HttpUrlRequestFactory.createFactory(getContext(), builder);
        assertTrue("Factory should be HttpUrlConnection/n.n.n.n@r but is "
                           + factory.getName(),
                   Pattern.matches(
                           "HttpUrlConnection/\\d+\\.\\d+\\.\\d+\\.\\d+@\\w+",
                           factory.getName()));
        // Load test library for starting the native test server.
        System.loadLibrary("cronet_tests");

        assertTrue(NativeTestServer.startNativeTestServer(getContext()));
        String url = NativeTestServer.getEchoHeaderURL(userAgentName);
        TestHttpUrlRequestListener listener = new TestHttpUrlRequestListener();
        HashMap<String, String> headers = new HashMap<String, String>();
        HttpUrlRequest request = factory.createRequest(
                url, 0, headers, listener);
        request.start();
        listener.blockForComplete();
        assertEquals(userAgentValue, listener.mResponseAsString);
        NativeTestServer.shutdownNativeTestServer();
    }

    @SmallTest
    @Feature({"Cronet"})
    public void testEnableHttpCache() {
        CronetEngine.Builder builder = new CronetEngine.Builder(null /*context*/);
        builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_DISABLED, 0);
        builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_IN_MEMORY, 0);
        try {
            builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_DISK, 0);
            fail("IllegalArgumentException must be thrown");
        } catch (IllegalArgumentException e) {
            assertEquals("Storage path must be set", e.getMessage());
        }
        try {
            builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_DISK_NO_HTTP, 0);
            fail("IllegalArgumentException must be thrown");
        } catch (IllegalArgumentException e) {
            assertEquals("Storage path must be set", e.getMessage());
        }

        // Create a new directory to hold the disk cache data.
        File dir = getContext().getDir("disk_cache_dir", Context.MODE_PRIVATE);
        String path = dir.getPath();
        builder.setStoragePath(path);
        builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_DISK, 100);
        builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_DISK_NO_HTTP, 100);
        try {
            builder.enableHttpCache(CronetEngine.Builder.HTTP_CACHE_IN_MEMORY, 0);
            fail("IllegalArgumentException must be thrown");
        } catch (IllegalArgumentException e) {
            assertEquals("Storage path must not be set", e.getMessage());
        }
        assertTrue(dir.delete());
    }
}
