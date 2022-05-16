set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES stream_parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    action.json
    cancel_all_after.json
    error.json
    error_response.json
    exec_inst.json
    exec_type.json
    execution.json
    execution_item.json
    field.json
    funding.json
    funding_item.json
    handshake.json
    instrument.json
    instrument_item.json
    liquidation.json
    liquidation_item.json
    liquidity_ind.json
    margin.json
    margin_item.json
    multi_leg_reporting_type.json
    ord_status.json
    ord_type.json
    order.json
    order_book_l2.json
    order_book_l2_item.json
    order_item.json
    position.json
    position_item.json
    quote.json
    quote_item.json
    settlement.json
    settlement_item.json
    settlement_type.json
    side.json
    state.json
    subscribe.json
    table.json
    time_in_force.json
    trade.json
    trade_item.json
    typ.json
    unsubscribe.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/bitmex/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-json-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT
  AUTOGEN_SOURCES
  NAMESPACE
  "roq/bitmex/json"
  OUTPUT_TYPE
  "json"
  FILE_TYPE
  "cpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

roq_gitignore(OUTPUT ".gitignore" SOURCES "${TARGET_NAME}" ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library("${TARGET_NAME}" STATIC ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)

target_link_libraries("${TARGET_NAME}" roq-core::roq-core roq-logging::roq-logging fmt::fmt-header-only)
