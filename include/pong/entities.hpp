#pragma once

#include <raylib.h>

namespace pong::entities {
struct Paddle {
    Rectangle rect;
};

struct Ball {
    Vector2 pos;
    Vector2 velocity;
    float radius;
};
} // namespace pong::entities