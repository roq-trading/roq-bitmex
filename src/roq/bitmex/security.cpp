/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/security.h"

#include <cassert>

#include <magic_enum.hpp>

#include "roq/core/binascii/hex.h"
#include "roq/core/crypto/hmac.h"
#include "roq/core/uri.h"

#include "roq/bitmex/flags.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

namespace {
static auto create_base_path() {
  core::URI uri(Flags::rest_uri());
  return uri.path;
}

static auto create_timestamp_secs(std::chrono::nanoseconds value) {
  return roq::format("{}"_fmt, std::chrono::duration_cast<std::chrono::seconds>(value).count());
}
}  // namespace

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), base_path(create_base_path()), key_(config.get_api_key()),
      hmac_(config.get_secret()) {
}

std::string Security::create_signature(
    std::chrono::nanoseconds expires,
    const core::http::Method &method,
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

std::string Security::create_headers(
    std::chrono::nanoseconds expires,
    const core::http::Method &method,
    const std::string_view &path,
    const std::string_view &body) {
  auto signature = create_signature(expires, method, path, body);
  return roq::format(
      "api-signature: {}\r\n"
      "api-expires: {}\r\n"
      "api-key: {}\r\n"_fmt,
      signature,
      std::chrono::duration_cast<std::chrono::seconds>(expires).count(),
      key_);
}

}  // namespace bitmex
}  // namespace roq
