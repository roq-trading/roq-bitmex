/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/crypto/hmac_sha256.hpp"

#include "roq/core/http/method.hpp"

namespace roq {
namespace bitmex {
namespace tools {

class Hasher final {
 public:
  Hasher(std::string_view const &key, std::string_view const &secret);

  Hasher(Hasher &&) = delete;
  Hasher(Hasher const &) = delete;

  std::string create_signature(
      std::chrono::nanoseconds expires,
      core::http::Method method,
      std::string_view const &path,
      std::string_view const &body);

  std::string create_headers(
      std::chrono::nanoseconds expires,
      core::http::Method method,
      std::string_view const &path,
      std::string_view const &body);

 private:
  const std::string key_;
  core::crypto::HMAC_SHA256 hmac_;
};

}  // namespace tools
}  // namespace bitmex
}  // namespace roq
