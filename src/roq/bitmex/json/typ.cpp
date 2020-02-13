/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/typ.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr Typ parse_FFC(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'C' &&
      name[4] == 'S' &&
      name[5] == 'X') {
    return Typ::FFCCSX;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_FFW(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'C' &&
      name[4] == 'S' &&
      name[5] == 'X') {
    return Typ::FFWCSX;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_F(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'F') {
    switch (name[2]) {
      case 'C':
        return parse_FFC(name);
      case 'W':
        return parse_FFW(name);
    }
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_MRC(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'X' &&
      name[4] == 'X' &&
      name[5] == 'X') {
    return Typ::MRCXXX;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_MRI(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'X' &&
      name[4] == 'X' &&
      name[5] == 'X') {
    return Typ::MRIXXX;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_MRR(const std::string_view& name) {
  if (name.length() == 6 &&
      name[3] == 'X' &&
      name[4] == 'X' &&
      name[5] == 'X') {
    return Typ::MRRXXX;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_M(const std::string_view& name) {
  if (name.length() >= 3 &&
      name[1] == 'R') {
    switch (name[2]) {
      case 'C':
        return parse_MRC(name);
      case 'I':
        return parse_MRI(name);
      case 'R':
        return parse_MRR(name);
    }
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_OC(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'E' &&
      name[3] == 'C' &&
      name[4] == 'C' &&
      name[5] == 'S') {
    return Typ::OCECCS;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_OP(const std::string_view& name) {
  if (name.length() == 6 &&
      name[2] == 'E' &&
      name[3] == 'C' &&
      name[4] == 'C' &&
      name[5] == 'S') {
    return Typ::OPECCS;
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_O(const std::string_view& name) {
  if (name.length() > 1) {
    switch (name[1]) {
      case 'C':
        return parse_OC(name);
      case 'P':
        return parse_OP(name);
    }
  }
  return Typ::UNKNOWN;
}

constexpr Typ parse_name(const std::string_view& name) {
  if (name.length() > 0) {
    switch (name[0]) {
      case 'F':
        return parse_F(name);
      case 'M':
        return parse_M(name);
      case 'O':
        return parse_O(name);
    }
  }
  return Typ::UNKNOWN;
}

static_assert(parse_name("FFCCSX") == Typ::FFCCSX);
static_assert(parse_name("FFWCSX") == Typ::FFWCSX);
static_assert(parse_name("MRCXXX") == Typ::MRCXXX);
static_assert(parse_name("MRIXXX") == Typ::MRIXXX);
static_assert(parse_name("MRRXXX") == Typ::MRRXXX);
static_assert(parse_name("OCECCS") == Typ::OCECCS);
static_assert(parse_name("OPECCS") == Typ::OPECCS);
}  // namespace

Typ parse_typ(const std::string_view& name) {
  auto result = parse_name(name);
  DLOG_IF(FATAL, result == Typ::UNKNOWN)(
      FMT_STRING("Unknown name=\"{}\""),
      name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
