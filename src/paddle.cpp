#include <algorithm>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>
#include <raylib.h>

namespace pong::paddle {
entities::Paddle init(Side side, float paddleWidth, float paddleLength,
                      int screenWidth, int screenHeight) {
    // TODO: use constants to define the starting positions
    Vector2 startPos{side == Side::Left ? paddleWidth * 3
                                        : screenWidth - (paddleWidth * 4),
                     (screenHeight / 2.0f) - (paddleLength / 2.0f)};

    return {.rect = {.x = startPos.x,
                     .y = startPos.y,
                     .width = paddleWidth,
                     .height = paddleLength}};
}

entities::Paddle move(entities::Paddle paddle, int direction, float speed,
                      float dt, int screenHeight) {
    paddle.rect.y += direction * speed * dt;
    paddle.rect.y =
        std::clamp(paddle.rect.y, 0.0f, screenHeight - paddle.rect.height);

    return paddle;
}
} // namespace pong::paddle