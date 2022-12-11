/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/web/http/method.hpp"

#include "roq/bitmex/config.hpp"

#include "roq/bitmex/tools/hasher.hpp"

namespace roq {
namespace bitmex {

class Security final {
 public:
  Security(Config const &, std::string_view const &account);

  Security(Security &&) = delete;
  Security(Security const &) = delete;

  std::string_view get_account() const { return account_; }

  std::string create_signature(
      std::chrono::nanoseconds expires,
      web::http::Method method,
      std::string_view const &path,
      std::string_view const &body);

  std::string create_headers(
      std::chrono::nanoseconds expires,
      web::http::Method method,
      std::string_view const &path,
      std::string_view const &body);

 private:
  const std::string account_;
  const std::string base_path;
  tools::Hasher hasher_;
};

}  // namespace bitmex
}  // namespace roq
