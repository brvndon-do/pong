add_library(pong_core STATIC paddle.cpp ball.cpp)

target_include_directories(pong_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(pong_core PRIVATE pong_warnings)
target_link_libraries(pong_core PUBLIC raylib)