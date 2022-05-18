/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/security.hpp"

#include "roq/core/uri.hpp"

#include "roq/bitmex/flags.hpp"

namespace roq {
namespace bitmex {

namespace {
auto create_base_path() {
  core::URI uri(Flags::rest_uri());
  return uri.path;
}
}  // namespace

Security::Security(Config const &config, std::string_view const &account)
    : account_(account), base_path(create_base_path()), hasher_(config.get_api_key(), config.get_secret()) {
}

std::string Security::create_signature(
    std::chrono::nanoseconds expires,
    core::http::Method method,
    std::string_view const &path,
    std::string_view const &body) {
  return hasher_.create_signature(expires, method, path, body);
}

std::string Security::create_headers(
    std::chrono::nanoseconds expires,
    core::http::Method method,
    std::string_view const &path,
    std::string_view const &body) {
  return hasher_.create_headers(expires, method, path, body);
}

}  // namespace bitmex
}  // namespace roq
