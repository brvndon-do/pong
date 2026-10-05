#pragma once

#include <pong/entities.hpp>

namespace pong::paddle {
enum class Side { Left, Right };

[[nodiscard]] entities::Paddle init(Side side, float paddleWidth,
                                    float paddleLength, int screenWidth,
                                    int screenHeight);

[[nodiscard]] entities::Paddle move(entities::Paddle paddle, int direction,
                                    float speed, float dt, int screenHeight);

} // namespace pong::paddle