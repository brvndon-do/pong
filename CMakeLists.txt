cmake_minimum_required(VERSION 3.25)

project(
    pong
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "basic pong game by Brandon Do"
)

enable_testing()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG 6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_library(pong_warnings INTERFACE)
target_compile_options(pong_warnings INTERFACE -Wall -Wextra -Wpedantic)

add_subdirectory(src/)
add_subdirectory(apps/)
add_subdirectory(tests/)
