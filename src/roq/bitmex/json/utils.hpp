/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/utils/patterns.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/bitmex/json/exec_inst.hpp"

namespace roq {
namespace bitmex {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{static_cast<uint64_t>(value * 1000)}; },
          [&](double value) { result = result_type{static_cast<uint64_t>(value * 1.0e3)}; },
          [&](std::string_view const &value) { result = utils::charconv::from_chars<result_type>(value, utils::charconv::Format::DATETIME); },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

inline json::ExecInst map(Mask<roq::ExecutionInstruction> execution_instructions) {
  // XXX support multiple?
  if (execution_instructions.has(roq::ExecutionInstruction::PARTICIPATE_DO_NOT_INITIATE)) {
    return json::ExecInst::PARTICIPATE_DO_NOT_INITIATE;
  }
  if (execution_instructions.has(roq::ExecutionInstruction::DO_NOT_INCREASE)) {
    return json::ExecInst::REDUCE_ONLY;
  }
  return {};
}

extern roq::Error guess_error(std::string_view const &message);

}  // namespace json
}  // namespace bitmex
}  // namespace roq
