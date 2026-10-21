#pragma once

#include <cstdint>
#include <pong/entities.hpp>

namespace pong::paddle {
enum class Side : std::uint8_t { Left, Right };

[[nodiscard]] entities::Paddle init(Side side, float paddleWidth,
                                    float paddleLength, int screenWidth,
                                    int screenHeight);

[[nodiscard]] entities::Paddle move(entities::Paddle paddle, int direction,
                                    float speed, float dt, int screenHeight);

} // namespace pong::paddle