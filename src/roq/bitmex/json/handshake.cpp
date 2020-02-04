/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/handshake.h"

#include "roq/bitmex/json/utils.h"

#include "roq/logging.h"

#ifdef VERSION
#undef VERSION
#endif

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  DOCS,
  INFO,
  TIMESTAMP,
  VERSION,
};

constexpr Field parse_d(auto& name) {
  if (name.compare("docs") == 0)
    return Field::DOCS;
  return Field::UNKNOWN;
}

constexpr Field parse_i(auto& name) {
  if (name.compare("info") == 0)
    return Field::INFO;
  return Field::UNKNOWN;
}

constexpr Field parse_t(auto& name) {
  if (name.compare("timestamp") == 0)
    return Field::TIMESTAMP;
  return Field::UNKNOWN;
}

constexpr Field parse_v(auto& name) {
  if (name.compare("version") == 0)
    return Field::VERSION;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  if (name.empty())
    return Field::UNKNOWN;
  switch (name[0]) {
    case 'd':
      return parse_d(name);
    case 'i':
      return parse_i(name);
    case 't':
      return parse_t(name);
    case 'v':
      return parse_v(name);
    default:
      return Field::UNKNOWN;
  }
}

static_assert(parse_name("docs") == Field::DOCS);
static_assert(parse_name("info") == Field::INFO);
static_assert(parse_name("timestamp") == Field::TIMESTAMP);
static_assert(parse_name("version") == Field::VERSION);

inline void update_field(
    auto& result,
    auto& key,
    auto& value) {
  auto field = parse_name(key);
  switch (field) {
    case Field::UNKNOWN:
      DLOG(FATAL)(
          FMT_STRING("Unknown key=\"{}\""),
          key);
      break;
    case Field::DOCS:
      update(result.docs, value);
      break;
    case Field::INFO:
      update(result.info, value);
      break;
    case Field::TIMESTAMP:
      update(result.timestamp, value);
      break;
    case Field::VERSION:
      update(result.version, value);
      break;
  }
}
}  // namespace

Handshake Handshake::parse(core::json::value_t& value) {
  Handshake result;
  for (auto [key, value] : std::get<core::json::object_t>(value))
    update_field(result, key, value);
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
