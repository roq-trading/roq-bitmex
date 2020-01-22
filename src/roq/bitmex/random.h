/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/crypto/hmac.h"

#include "roq/core/http/method.h"

namespace roq {
namespace bitmex {

class Random final {
 public:
  Random(
      const std::string_view& key,
      const std::string_view& secret);

  Random(const Random&) = delete;
  Random(Random&&) = delete;

  void operator=(const Random&) = delete;
  void operator=(Random&&) = delete;

  std::string create_signature(
      std::chrono::seconds timestamp,
      const core::http::Method& method,
      const std::string_view& path);

  std::string create_headers(
      std::chrono::seconds timestamp,
      const core::http::Method& method,
      const std::string_view& path);

 private:
  const std::string _key;
  core::crypto::HMAC_SHA256 _hmac;
};

}  // namespace bitmex
}  // namespace roq
