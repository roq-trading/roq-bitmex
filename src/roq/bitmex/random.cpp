/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/random.h"

#include <openssl/hmac.h>

#include <fmt/format.h>

#include <date/date.h>

#include <cinttypes>

#include <random>
#include <stdexcept>

#include "roq/core/binascii/hex.h"

#include "roq/core/crypto/hmac.h"

#if OPENSSL_VERSION_NUMBER < 0x10000000L
#error "Requires at least OpenSSL version 1.0"
#endif

namespace roq {
namespace bitmex {

static auto create_timestamp_secs(
    std::chrono::seconds value) {
  return fmt::format(
      "{}",
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
    std::chrono::seconds timestamp,
    const core::http::Method& method,
    const std::string_view& path) {
  auto t = create_timestamp_secs(timestamp);
  auto m = std::string_view(core::http::EnumNameMethod(method));
  _hmac.update(m);
  _hmac.update(path);
  _hmac.update(t);
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
    std::chrono::seconds timestamp,
    const core::http::Method& method,
    const std::string_view& path) {
  auto signature = create_signature(
      timestamp,
      method,
      path);
  return fmt::format(
      "api-signature: {}\r\n"
      "api-expires: {}\r\n"
      "api-key: {}\r\n",
      signature,
      timestamp.count(),
      _key);
}

}  // namespace bitmex
}  // namespace roq
