include(RoqAutogen)

set(TARGET_NAME ${PROJECT_NAME}-test)

set(SOURCES
    json_error_response.cpp
    json_margin.cpp
    json_execution.cpp
    json_funding.cpp
    json_instrument.cpp
    json_liquidation.cpp
    json_order_book_l2.cpp
    json_position.cpp
    json_quote.cpp
    json_settlement.cpp
    json_subscribe.cpp
    json_trade.cpp
    json_welcome.cpp
    main.cpp)

roq_gitignore(OUTPUT .gitignore SOURCES ${TARGET_NAME})

add_executable(${TARGET_NAME} ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-json Catch2::Catch2)

if(ROQ_BUILD_TYPE STREQUAL "Release")
  set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s)
endif()

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
