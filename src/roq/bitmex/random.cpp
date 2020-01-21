/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/random.h"

#include <openssl/hmac.h>

#include <fmt/format.h>

#include <date/date.h>

#include <cinttypes>

#include <random>
#include <stdexcept>

#include "roq/core/base64/base64.h"

#include "roq/core/crypto/hmac.h"

#if OPENSSL_VERSION_NUMBER < 0x10000000L
#error "Requires at least OpenSSL version 1.0"
#endif

namespace roq {
namespace bitmex {

namespace {
static std::random_device RANDOM_DEVICE;
static std::uniform_int_distribution<uint32_t> DISTRIBUTION;
}  // namespace

namespace {
static std::string create_number(uint64_t value) {
  // FIXME(thraneh): use charconv
  char buffer[64];
  snprintf(buffer, std::size(buffer), "%d", static_cast<int>(value));
  return buffer;
}
static std::string create_timestamp(std::chrono::nanoseconds value) {
  // FIXME(thraneh): use charconv
  date::sys_days days{std::chrono::duration_cast<date::days>(value)};
  date::year_month_day ymd(days);
  auto milliseconds = std::chrono::duration_cast<
  std::chrono::milliseconds>(value).count() % 86400000;
  auto hours = milliseconds / 3600000;
  milliseconds %= 3600000;
  auto minutes = milliseconds / 60000;
  milliseconds %= 60000;
  auto seconds = milliseconds / 1000;
  milliseconds %= 1000;
  char buffer[64];
  snprintf(buffer, std::size(buffer),
      "%04d%02d%02d-%02d:%02d:%02d.%03d",
      static_cast<int>(ymd.year()),
      static_cast<int>(static_cast<unsigned>(ymd.month())),
      static_cast<int>(static_cast<unsigned>(ymd.day())),
      static_cast<int>(hours),
      static_cast<int>(minutes),
      static_cast<int>(seconds),
      static_cast<int>(milliseconds));
  return buffer;
}
static std::string create_timestamp_secs(std::chrono::seconds value) {
  // FIXME(thraneh): use charconv
  char buffer[64];
  snprintf(buffer, std::size(buffer),
      "%" PRId64,
      value.count());
  return buffer;
}
}  // namespace

std::string Random::create_raw_data(
    std::chrono::nanoseconds sending_time,
    const std::string_view& msg_type,
    uint64_t msg_seq_num,
    const std::string_view& sender_comp_id,
    const std::string_view& target_comp_id,
    const std::string_view& password,
    const std::string_view& secret) {
  auto timestamp = create_timestamp(sending_time);
  auto number = create_number(msg_seq_num);
  auto key = core::base64::decode(
      secret.data(),
      secret.length(),
      false);
  char SOH = '\001';
  core::crypto::HMAC_SHA256 hmac(key.data(), key.size());
  hmac.update(timestamp);
  hmac.update(&SOH, 1);
  hmac.update(msg_type);
  hmac.update(&SOH, 1);
  hmac.update(number);
  hmac.update(&SOH, 1);
  hmac.update(sender_comp_id);
  hmac.update(&SOH, 1);
  hmac.update(target_comp_id);
  hmac.update(&SOH, 1);
  hmac.update(password);
  char buffer[32];
  auto length = hmac.digest(buffer, std::size(buffer));
  return core::base64::encode(buffer, length);
}

std::string Random::create_signature(
    std::chrono::seconds timestamp,
    const core::http::Method& method,
    const std::string_view& path,
    const std::string_view& secret) {
  auto t = create_timestamp_secs(timestamp);
  auto m = std::string_view(core::http::EnumNameMethod(method));
  auto key = core::base64::decode(
      secret.data(),
      secret.length(),
      false);
  core::crypto::HMAC_SHA256 hmac(key.data(), key.size());
  hmac.update(t);
  hmac.update(m);
  hmac.update(path);
  char buffer[32];
  auto length = hmac.digest(buffer, std::size(buffer));
  return core::base64::encode(buffer, length);
}

std::string Random::create_headers(
    std::chrono::seconds timestamp,
    const core::http::Method& method,
    const std::string_view& path,
    const std::string_view& key,
    const std::string_view& password,
    const std::string_view& secret) {
  auto signature = create_signature(
      timestamp,
      method,
      path,
      secret);
  return fmt::format(
      "CB-ACCESS-SIGN: {}\r\n"
      "CB-ACCESS-TIMESTAMP: {}\r\n"
      "CB-ACCESS-KEY: {}\r\n"
      "CB-ACCESS-PASSPHRASE: {}\r\n",
      signature,
      timestamp.count(),
      key,
      password);
}

}  // namespace bitmex
}  // namespace roq
