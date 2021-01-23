/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include "roq/service.h"

namespace roq {
namespace bitmex {

class Application final : public roq::Service {
 public:
  using roq::Service::Service;

 protected:
  int main(int, char **) override;
};

}  // namespace bitmex
}  // namespace roq
