/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <toml++/toml.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/logging.h"
#include "roq/server.h"

namespace roq {
namespace bitmex {

class Config final : public server::Config, public server::ConfigReader::Handler {
 public:
  explicit Config(const std::string_view &path);

  std::string get_master_account() const;

  auto get_api_key() const {
    using namespace roq::literals;
    if (accounts.size() != 1)
      throw RuntimeErrorException("More accounts not yet supported"_sv);
    return (*accounts.begin()).second.login;
  }
  auto get_secret() const {
    using namespace roq::literals;
    if (accounts.size() != 1)
      throw RuntimeErrorException("More accounts not yet supported"_sv);
    return (*accounts.begin()).second.secret;
  }

 protected:
  // server::Config
  void dispatch(server::Config::Handler &) const override;

  // server::ConfigReader::Handler
  void operator()(server::Symbols &&) override;
  void operator()(server::Account &&) override;
  void operator()(server::User &&) override;
  void operator()(const std::string_view &key, toml::node &) override;

 public:
  std::vector<server::User> users;
  server::Symbols symbols;
  absl::flat_hash_map<std::string, server::Account> accounts;
  std::string master_account_;
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
struct fmt::formatter<roq::bitmex::Config> : public roq::formatter {
  template <typename C>
  auto format(const roq::bitmex::Config &value, C &ctx) {
    using namespace roq::literals;
    return roq::format_to(
        ctx.out(),
        R"({{)"
        R"(symbols={}, )"
        R"(users=[{}], )"
        R"(accounts=[{}], )"
        R"(master_account="{}")"
        R"(}})"_sv,
        value.symbols,
        roq::join(value.users, ", "_sv),
        roq::join(value.accounts, ", "_sv),
        value.master_account_);
  }
};
