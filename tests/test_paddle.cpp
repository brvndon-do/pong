#include <catch2/catch_test_macros.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>

constexpr float dt{0.5f};
constexpr float speed{5.0f};

TEST_CASE("paddle::move moves vertically and stays on screen", "[paddle]") {
    pong::entities::Paddle paddle{pong::paddle::init(
        pong::paddle::Side::Left, 15.0f, 150.0f, pong::config::screen_width,
        pong::config::screen_height)};

    SECTION("test movement up/down") {
        // move down
        float y = paddle.rect.y;
        paddle = pong::paddle::move(paddle, 1, speed, dt,
                                    pong::config::screen_height);
        REQUIRE(paddle.rect.y == y + speed * dt);

        // move up
        paddle = pong::paddle::move(paddle, -1, speed, dt,
                                    pong::config::screen_height);
        REQUIRE(paddle.rect.y == y);
    }

    SECTION("test paddle boundaries") {
        // move down by large amount
        paddle = pong::paddle::move(paddle, 1, 100.0f, 100.0f,
                                    pong::config::screen_height);
        REQUIRE(paddle.rect.y ==
                pong::config::screen_height - paddle.rect.height);

        // move up by a large amount
        paddle = pong::paddle::move(paddle, -1, 100.0f, 100.0f,
                                    pong::config::screen_height);
        REQUIRE(paddle.rect.y == 0.0f);
    }
}