#include <format>
#include <optional>
#include <pong/ball.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/game.hpp>
#include <pong/paddle.hpp>
#include <random>
#include <raylib.h>
#include <string>

namespace cfg = pong::config;
namespace paddle = pong::paddle;
namespace ball = pong::ball;
namespace entities = pong::entities;
namespace game = pong::game;

namespace {
void drawCenterLine() {
    for (float y = 0; y < cfg::screen_height;
         y += (cfg::dash_gap + cfg::dash_length)) {
        DrawRectangleRec({.x = (static_cast<float>(cfg::screen_width) / 2.0f) -
                               (cfg::dash_width / 2),
                          .y = y,
                          .width = cfg::dash_width,
                          .height = cfg::dash_length},
                         cfg::dash_color);
    }
}

int getDirection(KeyboardKey up, KeyboardKey down) {
    return IsKeyDown(down) - IsKeyDown(up);
}

entities::Ball updateBall(entities::Ball ball, float dt,
                          entities::Paddle player1, entities::Paddle player2) {
    ball = ball::move(ball, dt);
    ball = ball::bounceWalls(ball, cfg::screen_height);
    ball = ball::bouncePaddle(ball, player1);
    ball = ball::bouncePaddle(ball, player2);

    return ball;
}
} // namespace

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 5);

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::screen_width, cfg::screen_height, cfg::window_title);
    SetTargetFPS(cfg::fps);

    game::Scoreboard scoreboard{};

    entities::Paddle player1{paddle::init(paddle::Side::Left, cfg::paddle_width,
                                          cfg::paddle_length, cfg::screen_width,
                                          cfg::screen_height)};

    entities::Paddle player2{
        paddle::init(paddle::Side::Right, cfg::paddle_width, cfg::paddle_length,
                     cfg::screen_width, cfg::screen_height)};

    entities::Ball ball{ball::init(
        cfg::ball_radius, static_cast<pong::ball::Direction>(dist(gen)),
        cfg::ball_speed, cfg::screen_width, cfg::screen_height)};

    while (!WindowShouldClose()) {
        const float dt{GetFrameTime()};
        const int player1Dir{getDirection(KEY_W, KEY_S)};
        const int player2Dir{getDirection(KEY_UP, KEY_DOWN)};

        // update
        player1 = paddle::move(player1, player1Dir, cfg::paddle_speed, dt,
                               cfg::screen_height);
        player2 = paddle::move(player2, player2Dir, cfg::paddle_speed, dt,
                               cfg::screen_height);

        ball = updateBall(ball, dt, player1, player2);

        std::optional<paddle::Side> scoringSide{
            game::getScoringSide(ball, cfg::screen_width)};
        if (scoringSide) {
            if (scoringSide == paddle::Side::Left) {
                ball = ball::init(cfg::ball_radius, ball::Direction::Left,
                                  cfg::ball_speed, cfg::screen_width,
                                  cfg::screen_height);
                ++scoreboard.player1;
            } else { // Side::Right
                ball = ball::init(cfg::ball_radius, ball::Direction::Right,
                                  cfg::ball_speed, cfg::screen_width,
                                  cfg::screen_height);
                ++scoreboard.player2;
            }
        }

        // draw
        BeginDrawing();

        ClearBackground(cfg::stage_background);

        drawCenterLine();

        DrawRectangleRec(player1.rect, cfg::paddle_color);
        DrawRectangleRec(player2.rect, cfg::paddle_color);
        DrawCircleV(ball.pos, ball.radius, cfg::ball_color);

        // scoreboard
        const std::string player1Score{
            std::format("Player 1: {}", scoreboard.player1)};
        const std::string player2Score{
            std::format("Player 2: {}", scoreboard.player2)};

        DrawText(player1Score.c_str(), cfg::text_font_gap, cfg::text_font_gap,
                 cfg::text_font_size, cfg::text_color);
        DrawText(player2Score.c_str(),
                 cfg::screen_width -
                     MeasureText(player2Score.c_str(), cfg::text_font_size) -
                     cfg::text_font_gap,
                 cfg::text_font_gap, cfg::text_font_size, cfg::text_color);

        EndDrawing();
    }

    CloseWindow();
}