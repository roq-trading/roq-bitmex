/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <string>
#include <string_view>

#include "roq/utils/mac/hmac.hpp"

#include "roq/web/http/method.hpp"

namespace roq {
namespace bitmex {
namespace tools {

struct Crypto final {
  Crypto(std::string_view const &key, std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

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
  using MAC = utils::mac::HMAC<utils::hash::SHA256>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  std::string const key_;
  MAC mac_;
  Digest digest_;
};

}  // namespace tools
}  // namespace bitmex
}  // namespace roq
