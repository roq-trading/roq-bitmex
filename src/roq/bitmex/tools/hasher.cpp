/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/tools/hasher.h"

#include <cassert>

#include <magic_enum.hpp>

#include "roq/core/binascii/hex.h"
#include "roq/core/crypto/hmac.h"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace tools {

namespace {
static auto create_timestamp_secs(std::chrono::nanoseconds value) {
  return fmt::format("{}"sv, std::chrono::duration_cast<std::chrono::seconds>(value).count());
}
}  // namespace

Hasher::Hasher(const std::string_view &key, const std::string_view &secret)
    : key_(key), hmac_(secret) {
}

std::string Hasher::create_signature(
    std::chrono::nanoseconds expires,
    core::http::Method method,
    const std::string_view &path,
    const std::string_view &body) {
  auto expires_ = create_timestamp_secs(expires);
  auto method_ = magic_enum::enum_name(method);
  hmac_.clear();
  hmac_.update(method_);
  hmac_.update(path);
  hmac_.update(expires_);
  if (!body.empty())
    hmac_.update(body);
  std::array<char, 32> buffer;
  auto length = hmac_.digest(buffer);
  assert(length == buffer.size());
  return core::binascii::Hex::encode(buffer);
}

std::string Hasher::create_headers(
    std::chrono::nanoseconds expires,
    core::http::Method method,
    const std::string_view &path,
    const std::string_view &body) {
  auto signature = create_signature(expires, method, path, body);
  return fmt::format(
      "api-signature: {}\r\n"
      "api-expires: {}\r\n"
      "api-key: {}\r\n"sv,
      signature,
      std::chrono::duration_cast<std::chrono::seconds>(expires).count(),
      key_);
}

}  // namespace tools
}  // namespace bitmex
}  // namespace roq
