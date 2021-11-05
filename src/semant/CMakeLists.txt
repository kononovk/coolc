list(APPEND COOLC_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/inheritance_graph.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/error.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/scope.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/semant.hpp)

list(APPEND COOLC_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/inheritance_graph.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/semant.cpp)

add_files()
