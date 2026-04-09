cmake_minimum_required(VERSION 3.21)

project(light_angel VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type"
      FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

function(light_angel_enable_warnings target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(
      ${target_name}
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion)
  endif()
endfunction()

add_subdirectory(src/fmt EXCLUDE_FROM_ALL)

add_library(light_angel_core
  src/hello.cpp
)

target_include_directories(light_angel_core
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(light_angel_core
  PRIVATE
    fmt::fmt
)

light_angel_enable_warnings(light_angel_core)

add_executable(light_angel
  src/main.cpp
)

target_link_libraries(light_angel
  PRIVATE
    light_angel_core
)

light_angel_enable_warnings(light_angel)
