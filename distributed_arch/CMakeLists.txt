cmake_minimum_required(VERSION 3.10)

project(jk_robotics)

# Use cmake prefix path to /usr/local/jk_robotics
set(CMAKE_INSTALL_PREFIX "/usr/local/jk_robotics" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(BUILD_SHARED_LIBS ON)

# Define build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "Supported build configurations" FORCE)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug, Release, or RelWithDebInfo)" FORCE)
endif()

# Set compiler flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG_BUILD")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DRELEASE_BUILD")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DRELWITHDEBINFO_BUILD")

# Add subdirectories
add_subdirectory(idl)
