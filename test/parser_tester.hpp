/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/bitmex/protocol/json/parser.hpp"

namespace roq {
namespace bitmex {

template <typename T>
struct ParserTester final : public protocol::json::Parser::Handler {
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
    auto res = protocol::json::Parser::dispatch(handler, message, buffers, {}, false);
    CHECK(res == true);
    CHECK(handler.found_ == true);
  }

 protected:
  explicit ParserTester(callback_type const &callback) : callback_{callback} {}

  virtual void operator()(Trace<protocol::json::Welcome> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Error> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Subscribe> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Unsubscribe> const &event) override { dispatch(event); }
  // public
  virtual void operator()(Trace<protocol::json::Instrument> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Quote> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::OrderBookL2> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Trade> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Funding> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Liquidation> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Settlement> const &event) override { dispatch(event); }
  // private
  virtual void operator()(Trace<protocol::json::CancelAllAfter> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Order> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Execution> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Margin> const &event) override { dispatch(event); }
  virtual void operator()(Trace<protocol::json::Position> const &event) override { dispatch(event); }

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
