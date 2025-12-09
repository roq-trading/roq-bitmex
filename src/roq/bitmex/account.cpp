/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitmex/account.hpp"

#include "roq/io/web/uri.hpp"

namespace roq {
namespace bitmex {

// === HELPERS ===

namespace {
auto create_crypto(auto &config, auto &name) {
  return tools::Crypto{config.get_api_key(name), config.get_secret(name)};
}
}  // namespace

// === IMPLEMENTATION ===

Account::Account(Settings const &, Config const &config, std::string_view const &name) : name{name}, crypto_{create_crypto(config, name)} {
}

std::string Account::create_signature(std::chrono::nanoseconds expires, web::http::Method method, std::string_view const &path, std::string_view const &body) {
  return crypto_.create_signature(expires, method, path, body);
}

std::string Account::create_headers(std::chrono::nanoseconds expires, web::http::Method method, std::string_view const &path, std::string_view const &body) {
  return crypto_.create_headers(expires, method, path, body);
}

}  // namespace bitmex
}  // namespace roq
