if(NOT CMAKE_CROSSCOMPILING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(NAME jkbuf_compiler COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/compiler_test.py
    $<TARGET_FILE:jkbuf> ${CMAKE_C_COMPILER} ${CMAKE_CXX_COMPILER} ${PROJECT_SOURCE_DIR}/runtime/include)
  # Compile the backend separately to enable test-only metadata-crash injection.
  add_executable(runtime_test runtime_test.cpp ../runtime/src/shared_memory.cpp)
  target_compile_definitions(runtime_test PRIVATE JK_RUNTIME_TESTING)
  target_include_directories(runtime_test PRIVATE ../runtime/include ${PROJECT_BINARY_DIR}/examples)
  target_link_libraries(runtime_test PRIVATE Threads::Threads rt)
  target_compile_options(runtime_test PRIVATE -Wall -Wextra -Wpedantic)
  add_dependencies(runtime_test example_schema)
  add_test(NAME local_runtime COMMAND runtime_test)
  add_test(NAME local_processes COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/process_test.py
    $<TARGET_FILE:nodemaster> $<TARGET_FILE:jk_publisher> $<TARGET_FILE:jk_subscriber>)
  set_tests_properties(jkbuf_compiler local_runtime local_processes PROPERTIES TIMEOUT 40)
endif()
