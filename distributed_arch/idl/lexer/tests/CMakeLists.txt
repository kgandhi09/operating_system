cmake_minimum_required(VERSION 3.10)

find_package(GTest REQUIRED)

add_executable(ridl_lexer_tests
    lexer_test.cpp
)

target_link_libraries(ridl_lexer_tests
PRIVATE
    ridl_lexer
    GTest::gtest
    GTest::gtest_main
)

target_compile_features(ridl_lexer_tests
PRIVATE
    cxx_std_20
)

target_compile_options(ridl_lexer_tests
PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

include(GoogleTest)

gtest_discover_tests(ridl_lexer_tests)

install(TARGETS ridl_lexer_tests
    RUNTIME DESTINATION bin
)
