add_library(ridl_ast INTERFACE)
target_include_directories(ridl_ast INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ridl_ast INTERFACE ridl_lexer)
