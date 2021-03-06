// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/live_sync/live_apps_sync_test.h"

#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/extensions/extension.h"

LiveAppsSyncTest::LiveAppsSyncTest(TestType test_type)
    : LiveSyncTest(test_type) {}

LiveAppsSyncTest::~LiveAppsSyncTest() {}

bool LiveAppsSyncTest::SetupClients() {
  if (!LiveSyncTest::SetupClients())
    return false;

  extension_helper_.Setup(this);
  return true;
}

bool LiveAppsSyncTest::AllProfilesHaveSameAppsAsVerifier() {
  // TODO(akalin): We may want to filter out non-apps for some tests.
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

void LiveAppsSyncTest::InstallApp(Profile* profile, int index) {
  std::string name = "fakeapp" + base::IntToString(index);
  return extension_helper_.InstallExtension(
      profile, name, Extension::TYPE_HOSTED_APP);
}

void LiveAppsSyncTest::InstallAppsPendingForSync(
    Profile* profile) {
  extension_helper_.InstallExtensionsPendingForSync(
      profile, Extension::TYPE_HOSTED_APP);
}
