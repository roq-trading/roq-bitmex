/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/tools/hasher.hpp"

#include <cassert>

#include <magic_enum.hpp>

#include "roq/core/binascii/hex.hpp"
#include "roq/core/crypto/hmac.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace tools {

namespace {
auto create_timestamp_secs(std::chrono::nanoseconds value) {
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
  if (!std::empty(body))
    hmac_.update(body);
  std::array<char, 32> buffer;
  auto length = hmac_.digest(buffer);
  assert(length == std::size(buffer));
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
