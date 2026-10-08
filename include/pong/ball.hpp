#pragma once

#include <pong/entities.hpp>

namespace pong::ball {
enum class Direction { LeftUp, Left, LeftDown, RightUp, Right, RightDown };

[[nodiscard]] entities::Ball init(float radius, Direction startingDirection,
                                  float speed, int screenWidth,
                                  int screenHeight);

[[nodiscard]] entities::Ball move(entities::Ball ball, float dt);

[[nodiscard]] entities::Ball bounceWalls(entities::Ball ball, int screenHeight);

[[nodiscard]] entities::Ball bouncePaddle(entities::Ball ball,
                                          entities::Paddle paddle);
} // namespace pong::ball