if(BUILD_TESTING)
  add_subdirectory(test)
endif()

if(BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()

set(TARGET_NAME ${PROJECT_NAME})

add_subdirectory(flags)

set(SOURCES
    account.cpp
    application.cpp
    config.cpp
    drop_copy.cpp
    gateway.cpp
    market_data.cpp
    order_entry.cpp
    order_update.cpp
    price_cache.cpp
    product.cpp
    settings.cpp
    shared.cpp
    web_socket.cpp
    main.cpp)

add_subdirectory(json)
add_subdirectory(tools)

include(RoqAutogen)

set(AUTOGEN_SCHEMAS market_data_state.json drop_copy_state.json web_socket_state.json)

roq_autogen(
  OUTPUT
  AUTOGEN_HEADERS
  NAMESPACE
  "roq/bitmex"
  OUTPUT_TYPE
  "state"
  FILE_TYPE
  "hpp"
  SOURCES
  ${AUTOGEN_SCHEMAS})

add_custom_target(autogen-headers ALL DEPENDS ${AUTOGEN_HEADERS})

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME} ${AUTOGEN_HEADERS})

add_executable(${TARGET_NAME} ${SOURCES})

add_dependencies(${TARGET_NAME} autogen-headers autogen-json-headers)

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-json
          ${PROJECT_NAME}-flags
          ${PROJECT_NAME}-tools
          roq-server::roq-server
          roq-server::roq-server-flags
          roq-web::roq-web
          roq-io::roq-io
          roq-core::roq-core
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-flags::roq-flags
          roq-api::roq-api
          fmt::fmt
          absl::flat_hash_map
          absl::flat_hash_set
          aklomp::base64
          ${UUID_LIBRARIES}
          ${RT_LIBRARIES})

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(
  ${TARGET_NAME}
  PRIVATE ROQ_PACKAGE_NAME="${TARGET_NAME}" ROQ_HOST="${ROQ_HOST}"
          ROQ_BUILD_VERSION="${GIT_REPO_VERSION}" ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}"
          ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
