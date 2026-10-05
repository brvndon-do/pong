#include <catch2/catch_test_macros.hpp>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>

constexpr float dt = 0.5f;
constexpr float speed = 5.0f;
constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;

TEST_CASE("Test Movement", "[paddle]") {
    pong::entities::Paddle paddle{pong::paddle::init(
        pong::paddle::Side::Left, 15.0f, 150.0f, screenWidth, screenHeight)};

    float y = paddle.rect.y;
    paddle = pong::paddle::move(paddle, 0.0f, speed, dt, screenHeight);
    REQUIRE(paddle.rect.y == y);

    paddle = pong::paddle::move(paddle, 1.0f, speed, dt, screenHeight);
    REQUIRE(paddle.rect.y == y + speed * dt);
}