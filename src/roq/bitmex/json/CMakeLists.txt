set(TARGET_NAME "${PROJECT_NAME}-json")

set(SOURCES
  parser.cpp
  utils.cpp
)

include(RoqAutogen)

roq_autogen(
  OUTPUT AUTOGEN_HEADERS
  PATTERN "*.json"
  NAMESPACE "roq/bitmex/json"
  OUTPUT_TYPE "json"
  FILE_TYPE "h")

add_custom_target(autogen-json-headers ALL
  DEPENDS ${AUTOGEN_HEADERS})

roq_autogen(
  OUTPUT AUTOGEN_SOURCES
  PATTERN "*.json"
  NAMESPACE "roq/bitmex/json"
  OUTPUT_TYPE "json"
  FILE_TYPE "cpp")

add_library("${TARGET_NAME}" STATIC
  ${SOURCES}
  ${AUTOGEN_SOURCES})

add_dependencies("${TARGET_NAME}" autogen-json-headers)
