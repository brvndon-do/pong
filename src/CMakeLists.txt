add_library(pong_core STATIC physics.cpp
    paddle.cpp)

target_include_directories(pong_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(pong_core PRIVATE pong_warnings)
target_link_libraries(pong_core PUBLIC raylib)