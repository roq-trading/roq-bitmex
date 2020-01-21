/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/http/method.h"

namespace roq {
namespace bitmex {

struct Random final {
  static std::string create_raw_data(
      std::chrono::nanoseconds sending_time,
      const std::string_view& msg_type,
      uint64_t msg_seq_num,
      const std::string_view& sender_comp_id,
      const std::string_view& target_comp_id,
      const std::string_view& password,
      const std::string_view& secret);

  static std::string create_signature(
      std::chrono::seconds timestamp,
      const core::http::Method& method,
      const std::string_view& path,
      const std::string_view& secret);

  static std::string create_headers(
      std::chrono::seconds timestamp,
      const core::http::Method& method,
      const std::string_view& path,
      const std::string_view& key,
      const std::string_view& password,
      const std::string_view& secret);
};

}  // namespace bitmex
}  // namespace roq
