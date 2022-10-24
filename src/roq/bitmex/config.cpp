/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/bitmex/config.hpp"

#include <utility>

#include "roq/logging.hpp"

#include "roq/mask.hpp"

#include "roq/bitmex/flags.hpp"

using namespace std::literals;

namespace roq {
namespace bitmex {

// === CONSTANTS ===

namespace {
auto const SUPPORTS = Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
    SupportType::STATISTICS,
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::ORDER,
    SupportType::ORDER_STATE,
    SupportType::TRADE,
    SupportType::POSITION,
};
auto const OMS_REQUEST_ID_TYPE = RequestIdType::BASE64;
}  // namespace

// === IMPLEMENTATION ===

Config::Config() {
  server::ConfigReader::parse_file(*this);
  log::info<1>("config={}"sv, *this);
}

Account const &Config::get_master_account() const {
  return master_account_;
}

void Config::dispatch(server::Config::Handler &handler) const {
  handler(Flags::exchange());
  handler(symbols);
  for (auto &iter : accounts)
    handler(iter.second);
  for (auto &user : users)
    handler(user);
  GatewaySettings gateway_settings{
      .supports = SUPPORTS,
      .mbp_max_depth = {},
      .mbp_tick_size_multiplier = NaN,
      .mbp_min_trade_vol_multiplier = NaN,
      .mbp_allow_remove_non_existing = {},
      .mbp_allow_price_inversion = Flags::mbp_allow_price_inversion(),
      .mbp_checksum = server::Flags::cache_mbp_checksum(),
      .oms_download_has_state = {},
      .oms_download_has_routing_id = {},
      .oms_request_id_type = OMS_REQUEST_ID_TYPE,
  };
  handler(gateway_settings);
  for (auto &iter : rate_limits)
    handler(iter.second);
}

void Config::operator()(server::Symbols &&symbols) {
  (*this).symbols = std::move(symbols);
}

void Config::operator()(server::Account &&account) {
  if (account.master)
    master_account_ = account.name;
  accounts.emplace(account.name, std::move(account));
}

void Config::operator()(server::User &&user) {
  users.emplace_back(std::move(user));
}

void Config::operator()(server::RateLimit &&rate_limit) {
  rate_limits.emplace(rate_limit.name, std::move(rate_limit));
}

void Config::operator()(std::string_view const &key, toml::node &) {
  log::warn(R"(Unexpected: key="{}")"sv, key);
}

}  // namespace bitmex
}  // namespace roq
