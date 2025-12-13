/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitmex/json/parser.hpp"

namespace roq {
namespace bitmex {

template <typename T>
struct ParserTester final : public json::Parser::Handler {
  using value_type = std::remove_cvref_t<T>;
  using callback_type = std::function<void(value_type const &)>;

  static void dispatch(callback_type const &callback, std::string_view const &message, size_t buffer_size, size_t max_depth) {
    core::json::BufferStack buffers{buffer_size, max_depth};
    // simple
    // XXX FIXME TODO catch2 block ???
    T obj{message, buffers};
    callback(obj);
    // parser
    // XXX FIXME TODO catch2 block ???
    ParserTester handler{callback};
    auto res = json::Parser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit ParserTester(callback_type const &callback) : callback_{callback} {}

  virtual void operator()(Trace<json::Welcome> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Error> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Subscribe> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Unsubscribe> const &event) override { dispatch(event); }
  // public
  virtual void operator()(Trace<json::Instrument> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Quote> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::OrderBookL2> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Trade> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Funding> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Liquidation> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Settlement> const &event) override { dispatch(event); }
  // private
  virtual void operator()(Trace<json::CancelAllAfter> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Order> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Execution> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Margin> const &event) override { dispatch(event); }
  virtual void operator()(Trace<json::Position> const &event) override { dispatch(event); }

  template <typename U>
  void dispatch(Trace<U> const &event) {
    if constexpr (std::is_invocable_v<callback_type, U>) {
      found_ = true;
      callback_(event);
    } else {
      FAIL();
    }
  }

 private:
  callback_type const callback_;
  bool found_ = false;
};

}  // namespace bitmex
}  // namespace roq
