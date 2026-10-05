cmake_minimum_required(VERSION 3.10)

add_library(ridl_lexer SHARED
    lexer.cpp
)

target_include_directories(ridl_lexer
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(ridl_lexer
PUBLIC
    cxx_std_20
)

target_compile_options(ridl_lexer
PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Installation
install(
    TARGETS ridl_lexer
    LIBRARY DESTINATION lib
)

install(
    DIRECTORY include/
    DESTINATION include
)


# tests
add_subdirectory(tests)
