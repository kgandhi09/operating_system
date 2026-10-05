cmake_minimum_required(VERSION 3.20)
project(basic_nodes LANGUAGES C CXX)

# Override this path when copying the example outside the nodeinfra checkout.
set(NODEINFRA_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH
    "Path to the nodeinfra source directory")
set(BUILD_TESTING OFF CACHE BOOL "Build nodeinfra tests")
set(JK_BUILD_EXAMPLES OFF CACHE BOOL "Build nodeinfra's other examples")
add_subdirectory("${NODEINFRA_SOURCE_DIR}" nodeinfra)

add_subdirectory(messages)

add_executable(temperature_publisher publisher.cpp)
target_link_libraries(temperature_publisher PRIVATE messages jk::jk_nodeinfra)

add_executable(temperature_subscriber subscriber.cpp)
target_link_libraries(temperature_subscriber PRIVATE messages jk::jk_nodeinfra)
