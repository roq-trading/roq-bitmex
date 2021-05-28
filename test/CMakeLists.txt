set(TARGET_NAME "${PROJECT_NAME}-test")

set(SOURCES json_error_response.cpp json_instrument.cpp main.cpp)

add_executable("${TARGET_NAME}" ${SOURCES})

target_link_libraries("${TARGET_NAME}" "${PROJECT_NAME}-json" roq-core::roq-core
                      roq-logging::roq-logging gtest_main ${RT_LIBRARIES})

target_compile_features("${TARGET_NAME}" PUBLIC cxx_std_17)

set_target_properties("${TARGET_NAME}" PROPERTIES LINK_FLAGS_RELEASE -s)

# run

add_test(NAME "${TARGET_NAME}" COMMAND "${TARGET_NAME}")
