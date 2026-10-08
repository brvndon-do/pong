#include <cmath>
#include <cstdlib>
#include <pong/ball.hpp>
#include <pong/entities.hpp>
#include <raylib.h>
#include <raymath.h>

namespace pong::ball {
entities::Ball init(float radius, Direction startingDirection, float speed,
                    int screenWidth, int screenHeight) {
    Vector2 startPos{screenWidth / 2.0f, screenHeight / 2.0f};
    Vector2 velocity{};

    switch (startingDirection) {
    case Direction::LeftUp:
        velocity = {-1.0f, -0.5f};
        break;
    case Direction::Left:
        velocity = {-1.0f, 0.0f};
        break;
    case Direction::LeftDown:
        velocity = {-1.0f, 0.5f};
        break;
    case Direction::RightUp:
        velocity = {1.0f, -0.5f};
        break;
    case Direction::Right:
        velocity = {1.0f, 0.0f};
        break;
    case Direction::RightDown:
        velocity = {1.0f, 0.5f};
        break;
    }

    return {.pos = startPos,
            .velocity = Vector2Normalize(velocity) * speed,
            .radius = radius};
}

// TODO: move into "update" function
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