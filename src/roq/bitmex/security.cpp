/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/security.h"

#include "roq/core/uri.h"

#include "roq/bitmex/flags.h"

namespace roq {
namespace bitmex {

namespace {
static auto create_base_path() {
  core::URI uri(Flags::rest_uri());
  return uri.path;
}
}  // namespace

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), base_path(create_base_path()),
      hasher_(config.get_api_key(), config.get_secret()) {
}

std::string Security::create_signature(
    std::chrono::nanoseconds expires,
    core::http::Method method,
    const std::string_view &path,
    const std::string_view &body) {
  return hasher_.create_signature(expires, method, path, body);
}

std::string Security::create_headers(
    std::chrono::nanoseconds expires,
    core::http::Method method,
    const std::string_view &path,
    const std::string_view &body) {
  return hasher_.create_headers(expires, method, path, body);
}

}  // namespace bitmex
}  // namespace roq
