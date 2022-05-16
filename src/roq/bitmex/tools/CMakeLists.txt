set(TARGET_NAME "${PROJECT_NAME}-tools")

set(SOURCES hasher.cpp)

add_library("${TARGET_NAME}" STATIC ${SOURCES})

target_link_libraries("${TARGET_NAME}" roq-core::roq-core fmt::fmt-header-only)
