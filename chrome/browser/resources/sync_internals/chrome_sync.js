// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var chrome = chrome || {};
// TODO(akalin): Add mocking code for e.g. chrome.send() so that we
// can test this without rebuilding chrome.
chrome.sync = chrome.sync || {};
(function () {

// This Event class is a simplified version of the one from
// event_bindings.js.
function Event() {
  this.listeners_ = [];
}

Event.prototype.addListener = function(listener) {
  this.listeners_.push(listener);
};

Event.prototype.removeListener = function(listener) {
  var i = this.findListener_(listener);
  if (i == -1) {
    return;
  }
  this.listeners_.splice(i, 1);
};

Event.prototype.hasListener = function(listener) {
  return this.findListener_(listener) > -1;
};

Event.prototype.hasListeners = function(listener) {
  return this.listeners_.length > 0;
};

// Returns the index of the given listener, or -1 if not found.
Event.prototype.findListener_ = function(listener) {
  for (var i = 0; i < this.listeners_.length; i++) {
    if (this.listeners_[i] == listener) {
      return i;
    }
  }
  return -1;
};

// Fires the event.  Called by the actual event callback.  Any
// exceptions thrown by a listener are caught and logged.
Event.prototype.fire = function() {
  var args = Array.prototype.slice.call(arguments);
  for (var i = 0; i < this.listeners_.length; i++) {
    try {
      this.listeners_[i].apply(null, args);
    } catch (e) {
      if (e instanceof Error) {
        // Non-standard, but useful.
        console.error(e.stack);
      } else {
        console.error(e);
      }
    }
  }
};

var events = [
  // Service events.
  'onSyncServiceStateChanged',

  // Notifier events.
  'onSyncNotificationStateChange',
  'onSyncIncomingNotification',

  // Manager events.
  'onChangesApplied',
  'onChangesComplete',
  'onSyncCycleCompleted',
  'onAuthError',
  'onUpdatedToken',
  'onPassphraseRequired',
  'onPassphraseAccepted',
  'onEncryptionComplete',
  'onMigrationNeededForTypes',
  'onInitializationComplete',
  'onPaused',
  'onResumed',
  'onStopSyncingPermanently',
  'onClearServerDataSucceeded',
  'onClearServerDataFailed'
];

for (var i = 0; i < events.length; ++i) {
  var event = events[i];
  chrome.sync[event] = new Event();
}

function makeAsyncFunction(name) {
  var callbacks = [];

  // Calls the function, assuming the last argument is a callback to be
  // called with the return value.
  var fn = function() {
    var args = Array.prototype.slice.call(arguments);
    callbacks.push(args.pop());
    chrome.send(name, args);
  };

  // Handle a reply, assuming that messages are processed in FIFO order.
  // Called by SyncInternalsUI::HandleJsMessageReply().
  fn.handleReply = function() {
    var args = Array.prototype.slice.call(arguments);
    // Remove the callback before we call it since the callback may
    // throw.
    var callback = callbacks.shift();
    callback.apply(null, args);
  };

  return fn;
}

var syncFunctions = [
  // Sync service functions.
  'getAboutInfo',

  // Notification functions.
  'getNotificationState',
  'getNotificationInfo',

  // Node lookup functions.
  'getRootNode',
  'getNodeById',
  'findNodesContainingString'
];

for (var i = 0; i < syncFunctions.length; ++i) {
  var syncFunction = syncFunctions[i];
  chrome.sync[syncFunction] = makeAsyncFunction(syncFunction);
}

})();
