#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/paddle.hpp>
#include <raylib.h>

namespace cfg = pong::config;
namespace paddle = pong::paddle;
namespace entities = pong::entities;

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

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::screen_width, cfg::screen_height, cfg::window_title);
    SetTargetFPS(cfg::fps);

    entities::Paddle player1 =
        paddle::init(paddle::Side::Left, cfg::paddle_width, cfg::paddle_length,
                     cfg::screen_width, cfg::screen_height);

    entities::Paddle player2 =
        paddle::init(paddle::Side::Right, cfg::paddle_width, cfg::paddle_length,
                     cfg::screen_width, cfg::screen_height);

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        const int player1Dir = getDirection(KEY_W, KEY_S);
        const int player2Dir = getDirection(KEY_UP, KEY_DOWN);

        // update
        player1 = paddle::move(player1, player1Dir, cfg::paddle_speed, dt,
                               cfg::screen_height);
        player2 = paddle::move(player2, player2Dir, cfg::paddle_speed, dt,
                               cfg::screen_height);

        // draw
        BeginDrawing();

        ClearBackground(cfg::stage_background);

        drawCenterLine();

        DrawRectangleRec(player1.rect, cfg::paddle_color);
        DrawRectangleRec(player2.rect, cfg::paddle_color);

        EndDrawing();
    }

    CloseWindow();
}