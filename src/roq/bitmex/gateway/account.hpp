/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/bitmex/gateway/config.hpp"
#include "roq/bitmex/gateway/settings.hpp"

#include "roq/bitmex/tools/crypto.hpp"

namespace roq {
namespace bitmex {
namespace gateway {

struct Account final {
  Account(Settings const &, Config const &, std::string_view const &name);

  Account(Account const &) = delete;

  std::string create_signature(std::chrono::nanoseconds expires, web::http::Method, std::string_view const &path, std::string_view const &body);

  std::string create_headers(std::chrono::nanoseconds expires, web::http::Method, std::string_view const &path, std::string_view const &body);

  std::string const name;

 private:
  // std::string const base_path;
  tools::Crypto crypto_;
};

}  // namespace gateway
}  // namespace bitmex
}  // namespace roq
