cmake_minimum_required(VERSION 3.31)

find_package(cmake-bare REQUIRED PATHS node_modules/cmake-bare)
find_package(cmake-bare-bundle REQUIRED PATHS node_modules/cmake-bare-bundle)
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-napi REQUIRED PATHS node_modules/cmake-napi)
find_package(cmake-npm REQUIRED PATHS node_modules/cmake-npm)

project(bare_kit LANGUAGES C CXX VERSION 1.1.0)

include(overrides.cmake)

install_node_modules()

if(WIN32)
    # For static runtime linking (/MT)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# Define build options for platform-specific subdirectories
option(ANDROID "Build for Android" OFF)
  option(DOTNET "Build for .NET" OFF)
option(APPLE "Build for Apple" OFF)

if(APPLE)
  enable_language(OBJC)
endif()

fetch_package("github:holepunchto/bare@1.13.1")

add_subdirectory(shared)


if(APPLE)
  add_subdirectory(apple)
endif()

if(ANDROID)
  add_subdirectory(android)
endif()

if(DOTNET)
  add_subdirectory(dotnet)
endif()

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
