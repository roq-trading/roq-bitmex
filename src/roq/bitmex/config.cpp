/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/bitmex/config.h"

#include <utility>

#include "roq/logging.h"

#include "roq/bitmex/options.h"

namespace roq {
namespace bitmex {

Config::Config(const std::string_view &path) {
  server::ConfigReader::parse(*this, path);
}

std::string Config::get_account() const {
  if (accounts.size() != 1)
    throw std::runtime_error("Only supporting 1 account");
  return (*accounts.begin()).first;
}

void Config::dispatch(server::Config::Handler &handler) const {
  handler(FLAGS_exchange);
  handler(symbols);
  for (auto iter : accounts) handler(iter.second);
  for (auto &user : users) handler(user);
}

void Config::operator()(server::Symbols &&symbols) {
  (*this).symbols = std::move(symbols);
}

void Config::operator()(Account &&account) {
  accounts.emplace(account.name, std::move(account));
}

void Config::operator()(User &&user) {
  users.emplace_back(std::move(user));
}

void Config::operator()(const std::string_view &key, cpptoml::base &) {
  LOG(WARNING)(R"(UNKNOWN KEY="{}")", key);
}

}  // namespace bitmex
}  // namespace roq
