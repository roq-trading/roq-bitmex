/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/random.h"

#include <fmt/format.h>

#include <cassert>

#include "roq/core/uri.h"
#include "roq/core/binascii/hex.h"
#include "roq/core/crypto/hmac.h"

#include "roq/bitmex/options.h"

namespace roq {
namespace bitmex {

static auto create_base_path() {
  core::URI uri(FLAGS_rest_uri);
  return uri.path;
}

static auto create_timestamp_secs(
    std::chrono::nanoseconds value) {
  return fmt::format(
      FMT_STRING("{}"),
      std::chrono::duration_cast<std::chrono::seconds>(value).count());
}

Random::Random(
    const std::string_view& key,
    const std::string_view& secret)
    : _base_path(create_base_path()),
      _key(key),
      _hmac(
          secret.data(),
          secret.length()) {
}

std::string Random::create_signature(
    std::chrono::nanoseconds expires,
    const core::http::Method& method,
    const std::string_view& path,
    const std::string_view& body) {
  auto expires_ = create_timestamp_secs(expires);
  auto method_ = std::string_view(core::http::EnumNameMethod(method));
  _hmac.clear();
  _hmac.update(method_);
  _hmac.update(path);
  _hmac.update(expires_);
  if (body.empty() == false)
    _hmac.update(body);
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
    std::chrono::nanoseconds expires,
    const core::http::Method& method,
    const std::string_view& path,
    const std::string_view& body) {
  auto signature = create_signature(
      expires,
      method,
      path,
      body);
  return fmt::format(
      FMT_STRING(
        "api-signature: {}\r\n"
        "api-expires: {}\r\n"
        "api-key: {}\r\n"),
      signature,
      std::chrono::duration_cast<std::chrono::seconds>(expires).count(),
      _key);
}

}  // namespace bitmex
}  // namespace roq
