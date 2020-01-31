/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/typ.h"

#include <cassert>

#include "roq/logging.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
constexpr auto parse_f(const std::string_view& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'C':
        if (name.compare("FFCCSX") == 0)
          return Typ::FFCCSX;
        break;
      case 'W':
        if (name.compare("FFWCSX") == 0)
          return Typ::FFWCSX;
        break;
    }
  }
  return Typ::UNKNOWN;
}

constexpr auto parse_m(const std::string_view& name) {
  if (name.length() >= 3) {
    switch (name.data()[2]) {
      case 'C':
        if (name.compare("MRCXXX") == 0)
          return Typ::MRCXXX;
        break;
      case 'I':
        if (name.compare("MRIXXX") == 0)
          return Typ::MRIXXX;
        break;
      case 'R':
        if (name.compare("MRRXXX") == 0)
          return Typ::MRRXXX;
        break;
    }
  }
  return Typ::UNKNOWN;
}

constexpr auto parse_o(const std::string_view& name) {
  if (name.length() >= 2) {
    switch (name.data()[1]) {
      case 'C':
        if (name.compare("OCECCS") == 0)
          return Typ::OCECCS;
        break;
      case 'P':
        if (name.compare("OPECCS") == 0)
          return Typ::OPECCS;
        break;
    }
  }
  return Typ::UNKNOWN;
}


constexpr auto parse_helper(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'F':
      return parse_f(name);
    case 'M':
      return parse_m(name);
    case 'O':
      return parse_o(name);
  }
  return Typ::UNKNOWN;
}

static_assert(parse_helper("FFCCSX") == Typ::FFCCSX);
static_assert(parse_helper("FFWCSX") == Typ::FFWCSX);
static_assert(parse_helper("MRCXXX") == Typ::MRCXXX);
static_assert(parse_helper("MRIXXX") == Typ::MRIXXX);
static_assert(parse_helper("MRRXXX") == Typ::MRRXXX);
static_assert(parse_helper("OCECCS") == Typ::OCECCS);
static_assert(parse_helper("OPECCS") == Typ::OPECCS);
}  // namespace

Typ parse_typ(const std::string_view& name) {
  auto result = parse_helper(name);
  DLOG_IF(FATAL, result == Typ::UNKNOWN)(
      "Unknown name=\"{}\"", name);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
