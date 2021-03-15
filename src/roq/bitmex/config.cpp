/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/bitmex/config.h"

#include <utility>

#include "roq/logging.h"

#include "roq/bitmex/flags.h"

using namespace roq::literals;

namespace roq {
namespace bitmex {

Config::Config(const std::string_view &path) {
  server::ConfigReader::parse(*this, path);
}

std::string Config::get_master_account() const {
  return master_account_;
}

void Config::dispatch(server::Config::Handler &handler) const {
  handler(Flags::exchange());
  handler(symbols);
  for (auto iter : accounts)
    handler(iter.second);
  for (auto &user : users)
    handler(user);
  server::Settings settings{};
  handler(settings);
}

void Config::operator()(server::Symbols &&symbols) {
  (*this).symbols = std::move(symbols);
}

void Config::operator()(server::Account &&account) {
  auto res = accounts.emplace(account.name, std::move(account));
  if (master_account_.empty())
    master_account_ = (*res.first).first;
}

void Config::operator()(server::User &&user) {
  users.emplace_back(std::move(user));
}

void Config::operator()(const std::string_view &key, cpptoml::base &) {
  LOG(WARNING)(R"(UNKNOWN KEY="{}")"_fmt, key);
}

}  // namespace bitmex
}  // namespace roq
