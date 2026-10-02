#include <raylib.h>
#include <pong/config.hpp>

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(
        pong::config::screen_width,
        pong::config::screen_height,
        pong::config::window_title
    );
    SetTargetFPS(pong::config::fps);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(pong::config::stage_background);

        EndDrawing();
    }

    CloseWindow();
}