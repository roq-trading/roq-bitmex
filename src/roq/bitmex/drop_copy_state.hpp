/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace bitmex {

enum class DropCopyState : uint8_t {
  UNDEFINED = 0,
  SUBSCRIBE,
  DONE,
};

}  // namespace bitmex
}  // namespace roq
