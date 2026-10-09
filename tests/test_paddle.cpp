#include <catch2/catch_test_macros.hpp>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>

constexpr float dt = 0.5f;
constexpr float speed = 5.0f;
constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;

TEST_CASE("paddle::move moves vertically and stays on screen", "[paddle]") {
    pong::entities::Paddle paddle{pong::paddle::init(
        pong::paddle::Side::Left, 15.0f, 150.0f, screenWidth, screenHeight)};

    SECTION("test movement up/down") {
        // move down
        float y = paddle.rect.y;
        paddle = pong::paddle::move(paddle, 1, speed, dt, screenHeight);
        REQUIRE(paddle.rect.y == y + speed * dt);

        // move up
        paddle = pong::paddle::move(paddle, -1, speed, dt, screenHeight);
        REQUIRE(paddle.rect.y == y);
    }

    SECTION("test paddle boundaries") {
        // move down by large amount
        paddle = pong::paddle::move(paddle, 1, 100.0f, 100.0f, screenHeight);
        REQUIRE(paddle.rect.y == screenHeight - paddle.rect.height);

        // move up by a large amount
        paddle = pong::paddle::move(paddle, -1, 100.0f, 100.0f, screenHeight);
        REQUIRE(paddle.rect.y == 0.0f);
    }
}