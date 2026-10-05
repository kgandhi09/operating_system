if(NOT CMAKE_CROSSCOMPILING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(NAME jkbuf_compiler COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/compiler_test.py
    $<TARGET_FILE:jkbuf> ${CMAKE_C_COMPILER} ${CMAKE_CXX_COMPILER} ${PROJECT_SOURCE_DIR}/runtime/include)
  # Compile the backend separately to enable test-only metadata-crash injection.
  add_executable(runtime_test runtime_test.cpp ../runtime/src/shared_memory.cpp)
  target_compile_definitions(runtime_test PRIVATE JK_RUNTIME_TESTING)
  target_link_libraries(runtime_test PRIVATE example_messages Threads::Threads rt)
  target_compile_options(runtime_test PRIVATE -Wall -Wextra -Wpedantic)
  add_test(NAME local_runtime COMMAND runtime_test)
  add_executable(node_app_test node_app_test.cpp)
  target_link_libraries(node_app_test PRIVATE example_messages jk_nodeinfra)
  target_compile_options(node_app_test PRIVATE -Wall -Wextra -Wpedantic)
  add_test(NAME node_app COMMAND node_app_test)
  set_tests_properties(node_app PROPERTIES TIMEOUT 30)
  add_test(NAME node_app_processes COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/node_app_process_test.py
    $<TARGET_FILE:nodemaster> $<TARGET_FILE:jk_class_publisher> $<TARGET_FILE:jk_class_subscriber>)
  set_tests_properties(node_app_processes PROPERTIES TIMEOUT 40)
  add_test(NAME local_processes COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/process_test.py
    $<TARGET_FILE:nodemaster> $<TARGET_FILE:jk_publisher> $<TARGET_FILE:jk_subscriber>)
  set_tests_properties(jkbuf_compiler local_runtime local_processes PROPERTIES TIMEOUT 40)
  add_test(NAME jkbuf_cmake_make COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_test.py
    ${CMAKE_COMMAND} ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
    ${CMAKE_C_COMPILER} ${CMAKE_CXX_COMPILER} "Unix Makefiles")
  set_tests_properties(jkbuf_cmake_make PROPERTIES TIMEOUT 180)
  find_program(JKBUF_NINJA_EXECUTABLE ninja)
  if(JKBUF_NINJA_EXECUTABLE)
    add_test(NAME jkbuf_cmake_ninja COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_test.py
      ${CMAKE_COMMAND} ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
      ${CMAKE_C_COMPILER} ${CMAKE_CXX_COMPILER} Ninja)
    set_tests_properties(jkbuf_cmake_ninja PROPERTIES TIMEOUT 180)
  endif()
endif()
