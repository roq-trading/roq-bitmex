/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/bitmex/account.hpp"

#include "roq/io/web/uri.hpp"

namespace roq {
namespace bitmex {

// === HELPERS ===

namespace {
auto create_base_path(auto &settings) {
  io::web::URI uri{settings.rest.uri};
  return std::string{uri.get_path()};
}
}  // namespace

// === IMPLEMENTATION ===

Account::Account(Settings const &settings, Config const &config, std::string_view const &name)
    : name{name}, base_path{create_base_path(settings)}, crypto_{config.get_api_key(), config.get_secret()} {
}

std::string Account::create_signature(
    std::chrono::nanoseconds expires,
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &body) {
  return crypto_.create_signature(expires, method, path, body);
}

std::string Account::create_headers(
    std::chrono::nanoseconds expires,
    web::http::Method method,
    std::string_view const &path,
    std::string_view const &body) {
  return crypto_.create_headers(expires, method, path, body);
}

}  // namespace bitmex
}  // namespace roq
