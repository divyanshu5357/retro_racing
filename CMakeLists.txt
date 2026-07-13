cmake_minimum_required(VERSION 3.20)
project(RetroRacer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
  message(FATAL_ERROR "RetroRacer currently uses the Windows Console API.")
endif()

add_executable(retro_racer src/main.cpp)
target_compile_definitions(retro_racer PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)

if(MSVC)
  target_compile_options(retro_racer PRIVATE /W4 /permissive-)
else()
  target_compile_options(retro_racer PRIVATE -Wall -Wextra -Wpedantic)
endif()
