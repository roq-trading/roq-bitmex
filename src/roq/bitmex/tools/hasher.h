/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/crypto/hmac.h"

#include "roq/core/http/method.h"

namespace roq {
namespace bitmex {
namespace tools {

class Hasher final {
 public:
  Hasher(const std::string_view &key, const std::string_view &secret);

  Hasher(Hasher &&) = delete;
  Hasher(const Hasher &) = delete;

  std::string create_signature(
      std::chrono::nanoseconds expires,
      core::http::Method method,
      const std::string_view &path,
      const std::string_view &body);

  std::string create_headers(
      std::chrono::nanoseconds expires,
      core::http::Method method,
      const std::string_view &path,
      const std::string_view &body);

 private:
  const std::string key_;
  core::crypto::HMAC_SHA256 hmac_;
};

}  // namespace tools
}  // namespace bitmex
}  // namespace roq
