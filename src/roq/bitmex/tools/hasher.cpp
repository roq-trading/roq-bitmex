/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/tools/hasher.hpp"

#include <cassert>

#include <magic_enum.hpp>

#include "roq/core/binascii/hex.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {
namespace tools {

// === HELPERS ===

namespace {
auto create_timestamp_secs(auto value) {
  return fmt::format("{}"sv, std::chrono::duration_cast<std::chrono::seconds>(value).count());
}
}  // namespace

// === IMPLEMENTATION ===

Hasher::Hasher(std::string_view const &key, std::string_view const &secret) : key_{key}, hmac_{secret} {
}

std::string Hasher::create_signature(
    std::chrono::nanoseconds expires,
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &body) {
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
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &body) {
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
