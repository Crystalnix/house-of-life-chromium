// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/live_sync/live_extensions_sync_test.h"

#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/extension.h"

LiveExtensionsSyncTest::LiveExtensionsSyncTest(TestType test_type)
    : LiveSyncTest(test_type) {}

LiveExtensionsSyncTest::~LiveExtensionsSyncTest() {}

bool LiveExtensionsSyncTest::SetupClients() {
  if (!LiveSyncTest::SetupClients())
    return false;

  extension_helper_.Setup(this);
  return true;
}

bool LiveExtensionsSyncTest::AllProfilesHaveSameExtensionsAsVerifier() {
  LiveSyncExtensionHelper::ExtensionStateMap
      verifier_extension_state_map(
          extension_helper_.GetExtensionStates(verifier()));
  for (int i = 0; i < num_clients(); ++i) {
    LiveSyncExtensionHelper::ExtensionStateMap
        extension_state_map(
            extension_helper_.GetExtensionStates(GetProfile(i)));
    if (extension_state_map != verifier_extension_state_map) {
      return false;
    }
  }
  return true;
}

void LiveExtensionsSyncTest::InstallExtension(Profile* profile, int index) {
  std::string name = "fakeextension" + base::IntToString(index);
  return extension_helper_.InstallExtension(
      profile, name, Extension::TYPE_EXTENSION);
}

void LiveExtensionsSyncTest::InstallExtensionsPendingForSync(
    Profile* profile) {
  extension_helper_.InstallExtensionsPendingForSync(
      profile, Extension::TYPE_EXTENSION);
}
