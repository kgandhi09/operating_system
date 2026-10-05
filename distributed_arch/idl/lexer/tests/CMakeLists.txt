add_executable(ridl_lexer_tests lexer_test.cpp)
target_link_libraries(ridl_lexer_tests PRIVATE ridl_lexer GTest::gtest GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(ridl_lexer_tests)
