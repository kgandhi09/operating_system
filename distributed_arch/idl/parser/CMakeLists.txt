cmake_minimum_required(VERSION 3.10)

add_library(ridl_parser SHARED
    parser.cpp
)

target_include_directories(ridl_parser
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ridl_parser
PUBLIC
    ridl_ast
    ridl_lexer
)

target_compile_features(ridl_parser
PUBLIC
    cxx_std_20
)

target_compile_options(ridl_parser
PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

install(
    TARGETS ridl_parser
    LIBRARY DESTINATION lib
)

install(
    DIRECTORY include/
    DESTINATION include
)
