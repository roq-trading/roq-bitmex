set(TARGET_NAME ${PROJECT_NAME}-protocol-json)

set(SOURCES encoder.cpp map.cpp parser.cpp utils.cpp)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS
    action.json
    cancel_all_after.json
    cancel_all_orders_ack.json
    cancel_order_ack.json
    error.json
    error_response.json
    exec_inst.json
    exec_type.json
    execution_data_item.json
    execution.json
    field.json
    funding_data_item.json
    funding.json
    instrument_data_item.json
    instrument.json
    liquidation_data_item.json
    liquidation.json
    liquidity_ind.json
    margin_data_item.json
    margin.json
    message.json
    multi_leg_reporting_type.json
    op.json
    order_book_l2_data_item.json
    order_book_l2.json
    order_data_item.json
    order.json
    ord_status.json
    ord_type.json
    position_data_item.json
    position.json
    quote_data_item.json
    quote.json
    settlement_data_item.json
    settlement.json
    settlement_type.json
    side.json
    state.json
    strategy.json
    subscribe.json
    subscribe_request.json
    table.json
    time_in_force.json
    trade_data_item.json
    trade.json
    typ.json
    unsubscribe.json
    welcome.json
    welcome_limit.json)

roq_autogen_hpp(
  OUTPUT
  AUTOGEN_HEADERS
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

add_custom_target(protocol-json-autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_autogen_cpp(
  OUTPUT
  AUTOGEN_SOURCES
  SOURCES
  ${AUTOGEN_SCHEMAS}
  TEMPLATE_DIR
  ${TEMPLATE_DIR}
  TEMPLATE_TYPE
  "json")

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS} ${AUTOGEN_SOURCES})

add_library(${TARGET_NAME} OBJECT ${SOURCES} ${AUTOGEN_SOURCES})

add_dependencies(${TARGET_NAME} protocol-json-autogen-headers)

target_link_libraries(${TARGET_NAME} PRIVATE roq-core::roq-core roq-codec::roq-codec roq-utils::roq-utils roq-logging::roq-logging fmt::fmt)
