// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_HTTP_MAC_SIGNATURE_H_
#define NET_HTTP_MAC_SIGNATURE_H_
#pragma once

#include <string>

#include "base/basictypes.h"
#include "base/gtest_prod_util.h"
#include "crypto/hmac.h"

namespace net {

// This class represents an HTTP MAC signature for use in the HTTP MAC
// Authentication scheme.  The current draft specification of this
// authentication scheme is located at the following URL:
//
//   http://tools.ietf.org/html/draft-hammer-oauth-v2-mac-token
//
class HttpMacSignature {
 public:
  HttpMacSignature();
  ~HttpMacSignature();

  // Returns whether this information is valid.
  bool AddStateInfo(const std::string& id,
                    const std::string& mac_key,
                    const std::string& mac_algorithm,
                    const std::string& issuer);

  // Returns whether this information is valid.
  bool AddHttpInfo(const std::string& method,
                   const std::string& request_uri,
                   const std::string& host,
                   int port);

  // Returns the value of the Authorization header for use in an HTTP request.
  std::string GenerateAuthorizationHeader();

 private:
  FRIEND_TEST_ALL_PREFIXES(HttpMacSignatureTest, GenerateHeaderString);
  FRIEND_TEST_ALL_PREFIXES(HttpMacSignatureTest, GenerateNormalizedRequest);
  FRIEND_TEST_ALL_PREFIXES(HttpMacSignatureTest, GenerateMAC);

  std::string GenerateHeaderString(const std::string& timestamp,
                                   const std::string& nonce);
  std::string GenerateNormalizedRequest(const std::string& timestamp,
                                        const std::string& nonce);
  std::string GenerateMAC(const std::string& timestamp,
                          const std::string& nonce);

  std::string id_;
  std::string mac_key_;
  crypto::HMAC::HashAlgorithm mac_algorithm_;
  std::string issuer_;

  std::string method_;
  std::string request_uri_;
  std::string host_;
  std::string port_;
  // TODO(abarth): body_hash_

  DISALLOW_COPY_AND_ASSIGN(HttpMacSignature);
};

}  // namespace net

#endif  // NET_HTTP_MAC_SIGNATURE_H_
