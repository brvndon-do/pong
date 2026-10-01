add_executable(pong main.cpp)

target_link_libraries(pong PRIVATE pong_core pong_warnings)