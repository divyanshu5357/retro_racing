cmake_minimum_required(VERSION 3.12)
project(RetroRacer CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the ncurses library on macOS
find_package(Curses REQUIRED)

# Add your source file (assuming it is named main.cpp)
add_executable(retroracer main.cpp)

# Include the ncurses directories and link the library
target_include_directories(retroracer PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(retroracer PRIVATE ${CURSES_LIBRARIES})