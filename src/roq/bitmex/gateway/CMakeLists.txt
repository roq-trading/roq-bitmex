set(TARGET_NAME ${PROJECT_NAME}-gateway)

set(SOURCES
    account.cpp
    api.cpp
    config.cpp
    controller.cpp
    drop_copy.cpp
    market_data.cpp
    order_entry.cpp
    order_update.cpp
    price_cache.cpp
    product.cpp
    shared.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCES})

add_dependencies(${TARGET_NAME} flags-autogen-headers protocol-json-autogen-headers)

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-flags
          ${PROJECT_NAME}-protocol-json
          ${PROJECT_NAME}-tools
          roq-api::roq-api
          roq-core::roq-core
          roq-io::roq-io
          roq-logging::roq-logging
          roq-server::roq-server
          roq-utils::roq-utils
          roq-web::roq-web
          fmt::fmt
          unordered_dense::unordered_dense)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${PROJECT_NAME}")

install(TARGETS ${TARGET_NAME})
