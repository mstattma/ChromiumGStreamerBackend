// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.test;

import android.os.Build;
import android.test.suitebuilder.annotation.SmallTest;
import android.view.KeyEvent;

import org.chromium.base.test.util.DisabledTest;
import org.chromium.base.test.util.Feature;
import org.chromium.base.test.util.MinAndroidSdkLevel;
import org.chromium.content.browser.input.ImeAdapter;
import org.chromium.content.browser.test.util.CallbackHelper;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Callable;

/**
 * Tests for the WebViewClient.onUnhandledKeyEvent() method.
 */
@MinAndroidSdkLevel(Build.VERSION_CODES.KITKAT)
public class AwContentsClientOnUnhandledKeyEventTest extends AwTestBase {
    private KeyEventTestAwContentsClient mContentsClient;
    private AwTestContainerView mTestContainerView;

    private static class UnhandledKeyEventHelper extends CallbackHelper {
        private final List<KeyEvent> mUnhandledKeyEventList = new ArrayList<>();

        public List<KeyEvent> getUnhandledKeyEventList() {
            return mUnhandledKeyEventList;
        }

        public void clearUnhandledKeyEventList() {
            mUnhandledKeyEventList.clear();
        }

        public void onUnhandledKeyEvent(KeyEvent event) {
            mUnhandledKeyEventList.add(event);
            notifyCalled();
        }
    }

    UnhandledKeyEventHelper mHelper;

    private class KeyEventTestAwContentsClient extends TestAwContentsClient {
        @Override
        public void onUnhandledKeyEvent(KeyEvent event) {
            mHelper.onUnhandledKeyEvent(event);
            super.onUnhandledKeyEvent(event);
        }
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mContentsClient = new KeyEventTestAwContentsClient();
        mHelper = new UnhandledKeyEventHelper();
        mTestContainerView = createAwTestContainerViewOnMainSync(mContentsClient);
    }

    /*
    @SmallTest
    @Feature({"AndroidWebView", "TextInput"})
     * http://crbug.com/538377
     * Chromium WebView currently sends unhandled events for all KeyUps and
     * composition codes (and always has), even when the textbox is handling
     * the corresponding KeyDowns.  This behavior violates Android's
     * InputEventConsistencyVerifier, although there are no currently known
     * broken use cases.  The correct fix for this is still unclear, but I'm
     * adding this new test as @DisabledTest to provide a tool for further
     * work.
    */
    @DisabledTest
    public void testTextboxConsumesKeyEvents() throws Throwable {
        enableJavaScriptOnUiThread(mTestContainerView.getAwContents());
        final String data = "<html><head></head><body><textarea id='textarea0'></textarea></body>"
                + "</html>";
        loadDataSync(mTestContainerView.getAwContents(), mContentsClient.getOnPageFinishedHelper(),
                data, "text/html", false);
        executeJavaScriptAndWaitForResult(mTestContainerView.getAwContents(), mContentsClient,
                "document.getElementById('textarea0').select();");

        int callCount;

        callCount = mHelper.getCallCount();
        dispatchDownAndUpKeyEvents(KeyEvent.KEYCODE_A);

        dispatchDownAndUpKeyEvents(KeyEvent.KEYCODE_DEL);

        // COMPOSITION_KEY is synthetically created for DOM's onkeydown() event while composing
        // text. We don't want to propagate it to WebViewClient.
        dispatchDownAndUpKeyEvents(ImeAdapter.COMPOSITION_KEY_CODE);

        // Alt-left should be unhandled but none of the previous events should be.
        dispatchDownAndUpKeyEvents(KeyEvent.KEYCODE_ALT_LEFT);

        mHelper.waitForCallback(callCount, 2);
        assertUnhandledDownAndUp(KeyEvent.KEYCODE_ALT_LEFT);
    }

    @SmallTest
    @Feature({"AndroidWebView", "TextInput"})
    public void testUnconsumedKeyEvents() throws Throwable {
        final String data = "<html><head></head><body>Plain page</body>"
                + "</html>";
        loadDataSync(mTestContainerView.getAwContents(), mContentsClient.getOnPageFinishedHelper(),
                data, "text/html", false);

        int callCount;

        callCount = mHelper.getCallCount();
        dispatchDownAndUpKeyEvents(KeyEvent.KEYCODE_A);
        mHelper.waitForCallback(callCount, 2);
        assertUnhandledDownAndUp(KeyEvent.KEYCODE_A);

        callCount = mHelper.getCallCount();
        dispatchDownAndUpKeyEvents(KeyEvent.KEYCODE_DEL);
        mHelper.waitForCallback(callCount, 2);
        assertUnhandledDownAndUp(KeyEvent.KEYCODE_DEL);

        callCount = mHelper.getCallCount();
        dispatchDownAndUpKeyEvents(KeyEvent.KEYCODE_ALT_LEFT);
        mHelper.waitForCallback(callCount, 2);
        assertUnhandledDownAndUp(KeyEvent.KEYCODE_ALT_LEFT);
    }

    private boolean dispatchKeyEvent(final KeyEvent event) throws Throwable {
        return runTestOnUiThreadAndGetResult(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                return mTestContainerView.dispatchKeyEvent(event);
            }
        });
    }

    private void dispatchDownAndUpKeyEvents(final int code) throws Throwable {
        dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, code));
        dispatchKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, code));
    }

    private void assertUnhandledDownAndUp(final int code) throws Throwable {
        List<KeyEvent> list = mHelper.getUnhandledKeyEventList();
        assertTrue("KeyEvent list: " + Arrays.deepToString(list.toArray()), list.size() == 2
                        && list.get(0).getAction() == KeyEvent.ACTION_DOWN
                        && list.get(0).getKeyCode() == code
                        && list.get(1).getAction() == KeyEvent.ACTION_UP
                        && list.get(1).getKeyCode() == code);

        mHelper.clearUnhandledKeyEventList();
    }
}

