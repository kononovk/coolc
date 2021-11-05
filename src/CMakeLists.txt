set(COOLC_SOURCES)
set(COOLC_HEADERS)

macro(add_files)
    set(COOLC_HEADERS ${COOLC_HEADERS} PARENT_SCOPE)
    set(COOLC_SOURCES ${COOLC_SOURCES} PARENT_SCOPE)
endmacro()


configure_file(config.hpp.in ${COOLC_BINARY_DIR}/include/coolc/config.hpp)

add_files(COOLC_HEADERS ${COOLC_BINARY_DIR}/include/coolc/config.hpp)

add_compile_options(${COOLC_COMPILE_OPTIONS})
add_link_options(${COOLC_LINK_OPTIONS})

add_subdirectory(lexer)
add_subdirectory(token)
add_subdirectory(parser)
add_subdirectory(util)
add_subdirectory(ast)
add_subdirectory(semant)

add_library(
        lib${PROJECT_NAME} STATIC
        ${COOLC_SOURCES}
        ${COOLC_HEADERS}
)

target_include_directories(lib${PROJECT_NAME}
        PUBLIC ${COOLC_BINARY_DIR}/include/ # for config.hpp
        PUBLIC ${COOLC_SOURCE_DIR}/src
        )
