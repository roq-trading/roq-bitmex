set(TARGET_NAME ${PROJECT_NAME}-fix-bridge)

set(SOURCES application.cpp settings.cpp main.cpp)

include(RoqAutogen)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_NAME}-flags
          ${PROJECT_NAME}-gateway
          roq-api::roq-api
          roq-fix::roq-fix
          roq-flags::roq-flags
          roq-logging::roq-logging
          roq-logging::roq-logging-flags
          roq-server::roq-server
          roq-server::roq-server-fix-bridge-flags
          fmt::fmt)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

target_compile_definitions(
  ${TARGET_NAME} PRIVATE ROQ_PACKAGE_NAME="${PROJECT_NAME}" ROQ_HOST="${ROQ_HOST}" ROQ_BUILD_VERSION="${GIT_REPO_VERSION}"
                         ROQ_GIT_DESCRIBE_HASH="${GIT_DESCRIBE_HASH}" ROQ_BUILD_NUMBER="${ROQ_BUILD_NUMBER}" ROQ_BUILD_TYPE="${ROQ_BUILD_TYPE}")

install(TARGETS ${TARGET_NAME})
