/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/random.h"

#include <fmt/format.h>

#include <cassert>

#include "roq/core/binascii/hex.h"

#include "roq/core/crypto/hmac.h"

namespace roq {
namespace bitmex {

static auto create_timestamp_secs(
    std::chrono::seconds value) {
  return fmt::format(
      FMT_STRING("{}"),
      value.count());
}

Random::Random(
    const std::string_view& key,
    const std::string_view& secret)
    : _key(key),
      _hmac(
          secret.data(),
          secret.length()) {
}

std::string Random::create_signature(
    std::chrono::seconds expires,
    const core::http::Method& method,
    const std::string_view& path) {
  auto expires_ = create_timestamp_secs(expires);
  auto method_ = std::string_view(core::http::EnumNameMethod(method));
  _hmac.clear();
  _hmac.update(method_);
  _hmac.update(path);
  _hmac.update(expires_);
  std::array<char, 32> buffer;
  auto length = _hmac.digest(
      buffer.data(),
      buffer.size());
  assert(length == buffer.size());
  return core::binascii::Hex::encode(
      buffer.data(),
      buffer.size());
}

std::string Random::create_headers(
    std::chrono::seconds expires,
    const core::http::Method& method,
    const std::string_view& path) {
  auto signature = create_signature(
      expires,
      method,
      path);
  return fmt::format(
      FMT_STRING(
        "api-signature: {}\r\n"
        "api-expires: {}\r\n"
        "api-key: {}\r\n"),
      signature,
      expires.count(),
      _key);
}

}  // namespace bitmex
}  // namespace roq
