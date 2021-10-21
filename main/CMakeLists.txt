add_compile_options(${COOLC_COMPILE_OPTIONS})
add_link_options(${COOLC_LINK_OPTIONS})
link_libraries(libcoolc)

set(COOLC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/util/util.cpp)
set(COOLC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/util/util.hpp
        )

add_executable(${PROJECT_NAME}
        ${COOLC_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${COOLC_HEADERS}
        )

# lexer binary for e2e tests
add_executable(lexer
        ${COOLC_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main_lexer.cpp
        ${COOLC_HEADERS}
        )

# parser binary for e2e tests
add_executable(parser
        ${COOLC_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main_parser.cpp
        ${COOLC_HEADERS}
        )

# semant binary for e2e tests
add_executable(semant
        ${COOLC_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main_semant.cpp
        ${COOLC_HEADERS}
        )

include_directories(PUBLIC ${COOLC_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
