/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/json/parser.h"

#include <stdexcept>

namespace roq {
namespace bitmex {
namespace json {

std::string_view Parser::find_type(const std::string_view& message) {
  // FIXME(thraneh): *must* ensure we only match on top-level
  // TODO(thraneh): allow whitespace around ':'
  auto begin = message.find("\"type\":\"");
  if (begin == message.npos)
    throw std::runtime_error("Can't find type");
  auto end = message.find_first_of('"', begin + 8);
  if (end == message.npos)
    throw std::runtime_error("Can't infer type");
  return std::string_view {
      message.data() + begin + 8,
      static_cast<size_t>(end - begin - 8)};
}

Parser::Type Parser::parse_type(const std::string_view& type) {
  if (type.empty())
    return Type::UNKNOWN;
  switch (type.data()[0]) {
    case 'a': {
      if (type.compare("activate") == 0) {
        return Type::ACTIVATE;
      }
      break;
    }
    case 'c': {
      if (type.compare("change") == 0) {
        return Type::CHANGE;
      }
      break;
    }
    case 'd': {
      if (type.compare("done") == 0) {
        return Type::DONE;
      }
      break;
    }
    case 'e': {
      if (type.compare("error") == 0) {
        return Type::ERROR;
      }
      break;
    }
    case 'h': {
      if (type.compare("heartbeat") == 0) {
        return Type::HEARTBEAT;
      }
      break;
    }
    case 'l': {
      if (type.length() >= 2) {
        switch (type.data()[1]) {
          case '2': {
            if (type.compare("l2update") == 0) {
              return Type::L2UPDATE;
            }
            break;
          }
          case 'a': {
            if (type.compare("last_match") == 0) {
              return Type::LAST_MATCH;
            }
            break;
          }
        }
      }
      break;
    }
    case 'm': {
      if (type.compare("match") == 0) {
        return Type::MATCH;
      }
      break;
    }
    case 'o': {
      if (type.compare("open") == 0) {
        return Type::OPEN;
      }
      break;
    }
    case 's': {
      if (type.length() >= 2) {
        switch (type.data()[1]) {
          case 'n': {
            if (type.compare("snapshot") == 0) {
              return Type::SNAPSHOT;
            }
            break;
          }
          case 't': {
            if (type.compare("status") == 0) {
              return Type::STATUS;
            }
            break;
          }
          case 'u': {
            if (type.compare("subscriptions") == 0) {
              return Type::SUBSCRIPTIONS;
            }
            break;
          }
        }
      }
      break;
    }
    case 'r': {
      if (type.compare("received") == 0) {
        return Type::RECEIVED;
      }
      break;
    }
    case 't': {
      if (type.compare("ticker") == 0) {
        return Type::TICKER;
      }
      break;
    }
    default: {
      break;
    }
  }
  return Type::UNKNOWN;
}

}  // namespace json
}  // namespace bitmex
}  // namespace roq
