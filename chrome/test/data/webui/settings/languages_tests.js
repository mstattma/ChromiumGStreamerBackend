// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

cr.define('settings-languages', function() {
  /**
   * Data-binds two Polymer properties using the property-changed events and
   * set/notifyPath API. Useful for testing components which would normally be
   * used together.
   * @param {!HTMLElement} el1
   * @param {!HTMLElement} el2
   * @param {string} property
   */
  function fakeDataBind(el1, el2, property) {
    var forwardChange = function(el, event) {
      if (event.detail.hasOwnProperty('path'))
        el.notifyPath(event.detail.path, event.detail.value);
      else
        el.set(property, event.detail.value);
    };
    // Add the listeners symmetrically. Polymer will prevent recursion.
    el1.addEventListener(property + '-changed', forwardChange.bind(null, el2));
    el2.addEventListener(property + '-changed', forwardChange.bind(null, el1));
  }

  suite('settings-languages', function() {
    function getFakePrefs() {
      var fakePrefs = [{
        key: 'intl.app_locale',
        type: chrome.settingsPrivate.PrefType.STRING,
        value: 'en-US',
      }, {
        key: 'intl.accept_languages',
        type: chrome.settingsPrivate.PrefType.STRING,
        value: 'en-US,sw',
      }, {
        key: 'spellcheck.dictionaries',
        type: chrome.settingsPrivate.PrefType.LIST,
        value: ['en-US'],
      }, {
        key: 'translate_blocked_languages',
        type: chrome.settingsPrivate.PrefType.LIST,
        value: ['en-US'],
      }];
      if (cr.isChromeOS) {
        fakePrefs.push({
          key: 'settings.language.preferred_languages',
          type: chrome.settingsPrivate.PrefType.STRING,
          value: 'en-US,sw',
        });
        fakePrefs.push({
          key: 'settings.language.preload_engines',
          type: chrome.settingsPrivate.PrefType.STRING,
          value: '_comp_ime_fgoepimhcoialccpbmpnnblemnepkkaoxkb:us::eng,' +
                 '_comp_ime_fgoepimhcoialccpbmpnnblemnepkkaoxkb:us:dvorak:eng',
        });
        fakePrefs.push({
          key: 'settings.language.enabled_extension_imes',
          type: chrome.settingsPrivate.PrefType.STRING,
          value: '',
        });
      }
      return fakePrefs;
    }

    var languageHelper;
    var languageSettingsPrivate;

    suiteSetup(function() {
      CrSettingsPrefs.deferInitialization = true;
    });

    setup(function() {
      var settingsPrefs = document.createElement('settings-prefs');
      var settingsPrivate = new settings.FakeSettingsPrivate(getFakePrefs());
      settingsPrefs.initialize(settingsPrivate);

      languageSettingsPrivate = new settings.FakeLanguageSettingsPrivate();
      languageSettingsPrivate.setSettingsPrefs(settingsPrefs);
      settings.languageSettingsPrivateApiForTest = languageSettingsPrivate;

      languageHelper = document.createElement('settings-languages');

      // Prefs would normally be data-bound to settings-languages.
      fakeDataBind(settingsPrefs, languageHelper, 'prefs');

      return languageHelper.whenReady();
    });

    test('languages model', function() {
      for (var i = 0; i < languageSettingsPrivate.languages.length;
           i++) {
        assertEquals(languageSettingsPrivate.languages[i].code,
                     languageHelper.languages.supported[i].code);
      }
      assertEquals(2, languageHelper.languages.enabled.length);
      assertEquals('en-US',
                   languageHelper.languages.enabled[0].language.code);
      assertEquals('sw',
                   languageHelper.languages.enabled[1].language.code);
      assertEquals('en', languageHelper.languages.translateTarget);

      // TODO(michaelpg): Test other aspects of the model.
    });

    test('modifying languages', function() {
      assertTrue(languageHelper.isLanguageEnabled('en-US'));
      assertTrue(languageHelper.isLanguageEnabled('sw'));
      assertFalse(languageHelper.isLanguageEnabled('en-CA'));

      languageHelper.enableLanguage('en-CA');
      assertTrue(languageHelper.isLanguageEnabled('en-CA'));
      languageHelper.disableLanguage('sw');
      assertFalse(languageHelper.isLanguageEnabled('sw'));

      // TODO(michaelpg): Test other modifications.
    });

    test('reorder languages', function() {
      languageHelper.enableLanguage('en-CA');
      assertEquals('en-US', languageHelper.languages.enabled[0].language.code);
      assertEquals('sw', languageHelper.languages.enabled[1].language.code);
      assertEquals('en-CA', languageHelper.languages.enabled[2].language.code);

      // Can move a language up.
      languageHelper.moveLanguage('en-CA', -1);
      assertEquals('en-US', languageHelper.languages.enabled[0].language.code);
      assertEquals('en-CA', languageHelper.languages.enabled[1].language.code);
      assertEquals('sw', languageHelper.languages.enabled[2].language.code);

      // Can move a language down.
      languageHelper.moveLanguage('en-US', 1);
      assertEquals('en-CA', languageHelper.languages.enabled[0].language.code);
      assertEquals('en-US', languageHelper.languages.enabled[1].language.code);
      assertEquals('sw', languageHelper.languages.enabled[2].language.code);

      // Moving the first language up has no effect.
      languageHelper.moveLanguage('en-CA', -1);
      assertEquals('en-CA', languageHelper.languages.enabled[0].language.code);
      assertEquals('en-US', languageHelper.languages.enabled[1].language.code);
      assertEquals('sw', languageHelper.languages.enabled[2].language.code);

      // Moving the last language down has no effect.
      languageHelper.moveLanguage('sw', 1);
      assertEquals('en-CA', languageHelper.languages.enabled[0].language.code);
      assertEquals('en-US', languageHelper.languages.enabled[1].language.code);
      assertEquals('sw', languageHelper.languages.enabled[2].language.code);
    });

    if (cr.isChromeOS) {
      test('modifying input methods', function() {
        assertEquals(2, languageHelper.languages.inputMethods.enabled.length);
        var inputMethods = languageHelper.getInputMethodsForLanguage('en-US');
        assertEquals(3, inputMethods.length);

        // We can remove one input method.
        var dvorak =
            '_comp_ime_fgoepimhcoialccpbmpnnblemnepkkaoxkb:us:dvorak:eng';
        languageHelper.removeInputMethod(dvorak);
        assertEquals(1, languageHelper.languages.inputMethods.enabled.length);

        // Enable Swahili.
        languageHelper.enableLanguage('sw');
        assertEquals(1, languageHelper.languages.inputMethods.enabled.length);

        // Add input methods for Swahili.
        var sw = '_comp_ime_abcdefghijklmnopqrstuvwxyzabcdefxkb:sw:sw';
        var swUS = '_comp_ime_abcdefghijklmnopqrstuvwxyzabcdefxkb:us:sw';
        languageHelper.addInputMethod(sw);
        languageHelper.addInputMethod(swUS);
        assertEquals(3, languageHelper.languages.inputMethods.enabled.length);

        // Disable Swahili. The Swahili-only keyboard should be removed.
        languageHelper.disableLanguage('sw');
        assertEquals(2, languageHelper.languages.inputMethods.enabled.length);

        // The US Swahili keyboard should still be enabled, because it supports
        // English which is still enabled.
        assertTrue(languageHelper.languages.inputMethods.enabled.some(
              function(inputMethod) {
                return inputMethod.id == swUS;
              }));
      });
    }
  });
});
