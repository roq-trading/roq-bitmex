/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/http/method.h"

#include "roq/bitmex/config.h"

#include "roq/bitmex/tools/hasher.h"

namespace roq {
namespace bitmex {

class Security final {
 public:
  Security(const Config &, const std::string_view &account);

  Security(Security &&) = delete;
  Security(const Security &) = delete;

  std::string_view get_account() const { return account_; }

  std::string create_signature(
      std::chrono::nanoseconds expires,
      core::http::Method method,
      const std::string_view &path,
      const std::string_view &body);

  std::string create_headers(
      std::chrono::nanoseconds expires,
      core::http::Method method,
      const std::string_view &path,
      const std::string_view &body);

 private:
  const std::string account_;
  const std::string base_path;
  tools::Hasher hasher_;
};

}  // namespace bitmex
}  // namespace roq
