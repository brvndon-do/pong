#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstdlib>
#include <pong/ball.hpp>
#include <pong/entities.hpp>

constexpr float ball_radius = 15.0f;
constexpr float speed = 5.0f;
constexpr int screenWidth = 1280;
constexpr int screenHeight = 720;

TEST_CASE("ball::bounceWalls", "[ball]") {
    const float starting_velocity_y = -10.0f;
    SECTION("upper boundary") {
        pong::entities::Ball ball{
            {0.0f, ball_radius}, {0.0f, starting_velocity_y}, ball_radius};

        ball = pong::ball::bounceWalls(ball, screenHeight);
        REQUIRE(ball.velocity.y == std::abs(starting_velocity_y));
    }

    SECTION("lower boundary") {
        pong::entities::Ball ball{{0.0f, screenHeight - ball_radius},
                                  {0.0f, std::abs(starting_velocity_y)},
                                  ball_radius};

        ball = pong::ball::bounceWalls(ball, screenHeight);
        REQUIRE(ball.velocity.y == starting_velocity_y);
    }
}

// TODO: other test cases for movement/paddle bounce