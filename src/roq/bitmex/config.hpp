/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <fmt/ranges.h>

#include <toml++/toml.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/logging.hpp"
#include "roq/server.hpp"

namespace roq {
namespace bitmex {

class Config final : public server::Config, public server::ConfigReader::Handler {
 public:
  Config();

  Account const &get_master_account() const;

  auto const &get_api_key() const {
    using namespace std::literals;
    if (std::size(accounts) != 1)
      throw RuntimeError("More accounts not yet supported"sv);
    return (*std::begin(accounts)).second.login;
  }
  auto const &get_secret() const {
    using namespace std::literals;
    if (std::size(accounts) != 1)
      throw RuntimeError("More accounts not yet supported"sv);
    return (*std::begin(accounts)).second.secret;
  }

 protected:
  // server::Config
  void dispatch(server::Config::Handler &) const override;

  // server::ConfigReader::Handler
  void operator()(server::Symbols &&) override;
  void operator()(server::Account &&) override;
  void operator()(server::User &&) override;
  void operator()(server::RateLimit &&) override;
  void operator()(std::string_view const &key, toml::node &) override;

 public:
  server::Users users;
  server::Symbols symbols;
  server::Accounts accounts;
  Account master_account_;
  server::RateLimits rate_limits;
};

/*
 * REST API
 * https://api-public.sandbox.pro.bitmex.com
 *
 * Websocket Feed
 * wss://ws-feed-public.sandbox.pro.bitmex.com
 *
 * FIX API
 * tcp+ssl://fix-public.sandbox.pro.bitmex.com:4198
 */

}  // namespace bitmex
}  // namespace roq

template <>
struct fmt::formatter<roq::bitmex::Config> {
  template <typename Context>
  constexpr auto parse(Context &context) {
    return std::begin(context);
  }
  template <typename Context>
  auto format(roq::bitmex::Config const &value, Context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(symbols={}, )"
        R"(accounts=[{}], )"
        R"(master_account="{}", )"
        R"(users=[{}], )"
        R"(rate_limits=[{}])"
        R"(}})"sv,
        value.symbols,
        fmt::join(value.accounts, ", "sv),
        value.master_account_,
        fmt::join(value.users, ", "sv),
        fmt::join(value.rate_limits, ", "sv));
  }
};
