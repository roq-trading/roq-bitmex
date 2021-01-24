/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <cpptoml.h>

#include <absl/container/flat_hash_map.h>

#include <fmt/format.h>

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

  std::string get_account() const;

  auto get_api_key() const {
    if (accounts.size() != 1)
      throw std::runtime_error("More accounts not yet supported");
    return (*accounts.begin()).second.login;
  }
  auto get_secret() const {
    if (accounts.size() != 1)
      throw std::runtime_error("More accounts not yet supported");
    return (*accounts.begin()).second.secret;
  }

 protected:
  // server::Config
  void dispatch(server::Config::Handler &handler) const override;

  // server::ConfigReader::Handler
  void operator()(server::Symbols &&symbols) override;
  void operator()(server::Account &&account) override;
  void operator()(server::User &&user) override;
  void operator()(const std::string_view &key, cpptoml::base &base) override;

 public:
  std::vector<server::User> users;
  server::Symbols symbols;
  absl::flat_hash_map<std::string, server::Account> accounts;
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
  template <typename C>
  constexpr auto parse(C &ctx) {
    return ctx.begin();
  }
  template <typename C>
  auto format(const roq::bitmex::Config &value, C &ctx) {
    // FIXME(thraneh): proper
    return format_to(
        ctx.out(),
        "{{"
        "users=[{}], "
        "accounts=..."
        "}}",
        fmt::join(value.users, ", "));
    // fmt::join(value.accounts, ", "));
  }
};
