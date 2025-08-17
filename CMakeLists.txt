cmake_minimum_required(VERSION 3.27)
project(souls_vision)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src/common/core)
add_subdirectory(src/common/game)
add_subdirectory(src/common/overlay)
add_subdirectory(src/er)
add_subdirectory(third_party)
