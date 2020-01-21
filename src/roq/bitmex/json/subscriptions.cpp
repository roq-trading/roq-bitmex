/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/subscriptions.h"

#include <cassert>
#include <utility>

#include "roq/core/json/array.h"
#include "roq/core/json/parser.h"

#include "roq/bitmex/json/utils.h"

namespace roq {
namespace bitmex {
namespace json {

namespace {
enum class Field {
  UNKNOWN,
  CHANNELS,
};

constexpr Field parse_c(auto& name) {
  if (name.compare("channels") == 0)
    return Field::CHANNELS;
  return Field::UNKNOWN;
}

constexpr Field parse_name(const std::string_view& name) {
  assert(name.empty() == false);
  switch (name.data()[0]) {
    case 'c':
      return parse_c(name);
  }
  return Field::UNKNOWN;
}

static_assert(parse_name("channels") == Field::CHANNELS);

struct Channel final {
  enum class Field {
    UNKNOWN,
    NAME,
    PRODUCT_IDS,
  };
  constexpr static Field parse_name(const std::string_view& name) {
    assert(name.empty() == false);
    switch (name.data()[0]) {
      case 'n':
        if (name.compare("name") == 0)
          return Field::NAME;
        break;
      case 'p':
        if (name.compare("product_ids") == 0)
          return Field::PRODUCT_IDS;
        break;
    }
    return Field::UNKNOWN;
  }
  static void parse(
      Subscriptions::channel_t& result,
      core::json::object_t&& object) {
    new (&result) std::remove_reference<decltype(result)>::type {};
    for (auto [key, value] : object) {
      auto field = parse_name(key);
      switch (field) {
        case Field::UNKNOWN: {
          break;
        }
        case Field::NAME: {
          update(result.name, value);
          break;
        }
        case Field::PRODUCT_IDS: {
          // not parsed
          break;
        }
      }
    }
  }
};
static_assert(Channel::parse_name("name") == Channel::Field::NAME);
static_assert(Channel::parse_name("product_ids") == Channel::Field::PRODUCT_IDS);
}  // namespace

Subscriptions Subscriptions::parse(
    const std::string_view& message,
    core::json::Buffer& buffer) {
  Subscriptions result;
  core::json::Parser parser(message);
  for (auto [key, value] : parser.root<core::json::object_t>()) {
    auto field = parse_name(key);
    switch (field) {
      case Field::UNKNOWN: {
        break;
      }
      case Field::CHANNELS: {
        core::json::Array channels(
            buffer,
            result.channels,
            value);
        for (auto [item, value] : channels) {
          Channel::parse(
              item,
              std::move(std::get<core::json::object_t>(value)));
        }
        break;
      }
    }
  }
  return result;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
