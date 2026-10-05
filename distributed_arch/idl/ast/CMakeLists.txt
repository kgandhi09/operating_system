cmake_minimum_required(VERSION 3.10)

add_library(ridl_ast INTERFACE)

target_include_directories(ridl_ast
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(ridl_ast INTERFACE cxx_std_20)

target_link_libraries(ridl_ast INTERFACE ridl_lexer)

install(DIRECTORY include/ DESTINATION include)
