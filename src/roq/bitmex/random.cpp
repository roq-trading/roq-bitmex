/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/random.h"

#include <fmt/format.h>

#include <cassert>

#include "roq/core/binascii/hex.h"
#include "roq/core/crypto/hmac.h"
#include "roq/core/uri.h"

#include "roq/bitmex/flags.h"

namespace roq {
namespace bitmex {

static auto create_base_path() {
  core::URI uri(Flags::rest_uri());
  return uri.path;
}

static auto create_timestamp_secs(std::chrono::nanoseconds value) {
  return fmt::format(
      "{}", std::chrono::duration_cast<std::chrono::seconds>(value).count());
}

Random::Random(const std::string_view &key, const std::string_view &secret)
    : base_path(create_base_path()), key_(key),
      hmac_(secret.data(), secret.length()) {
}

std::string Random::create_signature(
    std::chrono::nanoseconds expires,
    const core::http::Method &method,
    const std::string_view &path,
    const std::string_view &body) {
  auto expires_ = create_timestamp_secs(expires);
  auto method_ = std::string_view(core::http::EnumNameMethod(method));
  hmac_.clear();
  hmac_.update(method_);
  hmac_.update(path);
  hmac_.update(expires_);
  if (body.empty() == false)
    hmac_.update(body);
  std::array<char, 32> buffer;
  auto length = hmac_.digest(buffer.data(), buffer.size());
  assert(length == buffer.size());
  return core::binascii::Hex::encode(buffer.data(), buffer.size());
}

std::string Random::create_headers(
    std::chrono::nanoseconds expires,
    const core::http::Method &method,
    const std::string_view &path,
    const std::string_view &body) {
  auto signature = create_signature(expires, method, path, body);
  return fmt::format(
      "api-signature: {}\r\n"
      "api-expires: {}\r\n"
      "api-key: {}\r\n",
      signature,
      std::chrono::duration_cast<std::chrono::seconds>(expires).count(),
      key_);
}

}  // namespace bitmex
}  // namespace roq
