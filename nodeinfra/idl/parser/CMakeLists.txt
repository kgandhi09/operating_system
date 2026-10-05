add_library(ridl_parser STATIC parser.cpp)
target_include_directories(ridl_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ridl_parser PUBLIC ridl_ast ridl_lexer)
target_compile_options(ridl_parser PRIVATE -Wall -Wextra -Wpedantic)
