#pragma once

#include <pong/entities.hpp>
#include <pong/paddle.hpp>

namespace pong::ball {
[[nodiscard]] entities::Ball init(float radius, paddle::Side side, float angle,
                                  float speed, int screenWidth,
                                  int screenHeight);

[[nodiscard]] entities::Ball move(entities::Ball ball, float dt);

[[nodiscard]] entities::Ball bounceWalls(entities::Ball ball, int screenHeight);

[[nodiscard]] entities::Ball bouncePaddle(entities::Ball ball,
                                          entities::Paddle paddle);
} // namespace pong::ball