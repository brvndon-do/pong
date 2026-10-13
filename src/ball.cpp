#include <cmath>
#include <pong/ball.hpp>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>
#include <raylib.h>
#include <raymath.h>

namespace pong::ball {
entities::Ball init(float radius, paddle::Side side, float angle, float speed,
                    int screenWidth, int screenHeight) {
    Vector2 startPos{screenWidth / 2.0f, screenHeight / 2.0f};
    Vector2 velocity{std::cos(angle) *
                         (side == paddle::Side::Left ? -1.0f : 1.0f),
                     std::sin(angle)};

    return {.pos = startPos, .velocity = velocity * speed, .radius = radius};
}

entities::Ball move(entities::Ball ball, float dt) {
    ball.pos += ball.velocity * dt;

    return ball;
}

entities::Ball bounceWalls(entities::Ball ball, int screenHeight) {
    if (ball.pos.y - ball.radius <= 0.0f)
        ball.velocity.y = std::abs(ball.velocity.y);
    else if (ball.pos.y + ball.radius >= screenHeight)
        ball.velocity.y = -std::abs(ball.velocity.y);

    return ball;
}

entities::Ball bouncePaddle(entities::Ball ball, entities::Paddle paddle) {
    if (CheckCollisionCircleRec(ball.pos, ball.radius, paddle.rect)) {
        const float paddleCenterX{paddle.rect.x + (paddle.rect.width / 2.0f)};
        ball.velocity.x = ball.pos.x < paddleCenterX
                              ? -std::abs(ball.velocity.x)
                              : std::abs(ball.velocity.x);
    }

    return ball;
}
} // namespace pong::ball