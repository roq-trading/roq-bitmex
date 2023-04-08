# Change Log

All notable changes will be documented in this file.

## Head

### Fixed

* Broken price-caching for L2 books (#339)

## 0.9.3 &ndash; 2023-03-20

## 0.9.2 &ndash; 2023-02-22

## 0.9.1 &ndash; 2023-01-12

## 0.9.0 &ndash; 2022-12-22

## 0.8.9 &ndash; 2022-11-14

## 0.8.8 &ndash; 2022-10-04

## 0.8.7 &ndash; 2022-08-22

## 0.8.6 &ndash; 2022-07-18

## 0.8.5 &ndash; 2022-06-06

### Changed

* Market data support for `--net_disconnect_on_idle_timeout`.

### Fixed

* Symbol filtering was not used (#223)

## 0.8.4 &ndash; 2022-05-14

### Fixed

* Exchange time was missing from MbP (#211)

## 0.8.3 &ndash; 2022-03-22

## 0.8.2 &ndash; 2022-02-18

## 0.8.1 &ndash; 2022-01-16

## 0.8.0 &ndash; 2022-01-12

## 0.7.9 &ndash; 2021-12-08

### Added

* New flag to toggle use of WebSocket for order management (#112)

## 0.7.8 &ndash; 2021-11-02

### Fixed

* Resubscription wasn't implemented (#80)

### Added

* Add exchange sequence number to `MarketByPrice` and `MarketByOrder` (#101)
* Add `max_trade_vol` and `trade_vol_step_size` to ReferenceData (#100)

### Changed

* Move cache utilities to API (#111)
* Interface to support binary data from web::socket
* Change default WS end-point (#106)
* ReferenceData currencies should follow FX conventions (#99)
* Replace `snapshot` (bool) with `update_type` (UpdateType) (#97)
* Moved signature handling to tools library (chore)
* Allow "market data"-only operation (#96)

### Removed

* Remove custom literals (#110)
* Remove external rate-limiter mirroring from the REST connection (#83)

## 0.7.7 &ndash; 2021-09-20

### Changed

* Added parsing of "Rate limit exceeded, retry in 1 seconds." (#64)
* Added HTTP `request_id` (#55)
* `NetworkError` now used to populate `OrderAck` fields (#59)

## 0.7.6 &ndash; 2021-09-02

### Changed

* Two different order-updates based on order-entry or drop-copy (#25)
* Use web-safe "base64" encoding for ClOrdID (#43)
* Download orders (#39)
* Parse error messages (#32)
* New order management interface (#25)
* OrderAck.error is now **guessed** based on reject reason.
  NOTE! This is **best effort** and preferably not to be relied upon.
* OrderEntry will now parse response message for all HTTP status code 4xx.
  (BitMEX documentation only lists 400, 401, 403, 404, but have seen 409 as well.)

### Removed

* The `--rest_allow_order_updates` flag is no longer needed (#44)

## 0.7.5 &ndash; 2021-08-08

### Added

* `StatisticsType::FUNDING_RATE_PREDICTION`

## 0.7.4 &ndash; 2021-07-20

## 0.7.3 &ndash; 2021-07-06

### Fixed

* JSON PositionItem.posState was incorrectly configured as double

### Changed

* `StatisticsType::FUNDING_RATE` is now populated from the `instrument` channel (was previously
  sourced from the `funding` channel)

## 0.7.2 &ndash; 2021-06-20

### Changed

* HTTP response body is now parsed for status={400,401,403,404} and error message
  is propagated through OrderAck

## 0.7.1 &ndash; 2021-05-30

### Added

* New statistics fields

### Fixed

* Partially fixed 4xx HTTP response issues

## 0.7.0 &ndash; 2021-04-15

### Added

* Multi-account support

### Changed

* Streams to support load-balancing

## 0.6.1 &ndash; 2021-02-19

## 0.6.0 &ndash; 2021-02-02

## 0.5.0 &ndash; 2020-12-04

### Changed

* `PositionUpdate` now uses side == Side::UNDEFINED rather than
  trying to invent buy/sell

## 0.4.5 &ndash; 2020-11-09

## 0.4.4 &ndash; 2020-09-20

## 0.4.3 &ndash; 2020-09-02

## 0.4.2 &ndash; 2020-07-27

### Removed

* Automake support

## 0.4.1 &ndash; 2020-07-17

## 0.4.0 &ndash; 2020-06-30

## 0.3.9 &ndash; 2020-06-09

## 0.3.8 &ndash; 2020-06-06

## 0.3.7 &ndash; 2020-05-27

## 0.3.6 &ndash; 2020-05-02

## 0.3.5 &ndash; 2020-04-22

## 0.3.4 &ndash; 2020-04-08

## 0.3.3 &ndash; 2020-03-04
