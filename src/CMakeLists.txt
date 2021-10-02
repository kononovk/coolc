set(COOLC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
set(COOLC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/token.hpp)

macro(add_files)
    set(COOLC_HEADERS ${COOLC_HEADERS} PARENT_SCOPE)
    set(COOLC_SOURCES ${COOLC_SOURCES} PARENT_SCOPE)
endmacro()

add_compile_options(${YACLIB_COMPILE_OPTIONS})
add_link_options(${YACLIB_LINK_OPTIONS})

add_subdirectory(lexer)

# TODO: add_library ?!
#[[ add_library(
        ${PROJECT_NAME} STATIC
        ${COOLC_SOURCES}
)]]
add_executable(
        ${PROJECT_NAME}
        ${COOLC_HEADERS}
        ${COOLC_SOURCES}
)
