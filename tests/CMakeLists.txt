include (FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(Catch2)

add_executable(pong_tests test_config.cpp test_paddle.cpp)

target_link_libraries(pong_tests PRIVATE pong_core pong_warnings Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(pong_tests)