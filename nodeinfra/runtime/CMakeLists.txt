if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "The local shared-memory runtime currently requires Linux")
endif()
add_library(jk_message_headers INTERFACE)
add_library(jk::jk_message_headers ALIAS jk_message_headers)
target_include_directories(jk_message_headers INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(jk_nodeinfra STATIC src/shared_memory.cpp src/node_app.cpp)
add_library(jk::jk_nodeinfra ALIAS jk_nodeinfra)
target_link_libraries(jk_nodeinfra PUBLIC jk_message_headers Threads::Threads rt)
target_compile_features(jk_nodeinfra PUBLIC cxx_std_20)
target_compile_options(jk_nodeinfra PRIVATE -Wall -Wextra -Wpedantic)
add_executable(nodemaster src/nodemaster.cpp)
target_link_libraries(nodemaster PRIVATE jk_nodeinfra)
install(TARGETS jk_nodeinfra jk_message_headers nodemaster EXPORT jk_nodeinfraTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT jk_nodeinfraTargets NAMESPACE jk:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jk_nodeinfra)
include(CMakePackageConfigHelpers)
configure_package_config_file(jk_nodeinfraConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/jk_nodeinfraConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jk_nodeinfra)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/jk_nodeinfraConfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jk_nodeinfra)
install(FILES ../cmake/BuildJkbuf.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jk_nodeinfra)
