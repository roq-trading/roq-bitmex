set(TARGET_NAME ${PROJECT_NAME}-tools)

set(SOURCES crypto.cpp)

add_library(${TARGET_NAME} OBJECT ${SOURCES})

target_link_libraries(${TARGET_NAME} roq-core::roq-core fmt::fmt)
