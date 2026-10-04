#include <pong/config.hpp>
#include <raylib.h>

namespace cfg = pong::config;

void drawCenterLine(int screenWidth, int screenHeight) {
  for (int y = 0; y < screenHeight; y += (cfg::dash_gap + cfg::dash_length)) {
    DrawRectangleRec(
        {.x = (static_cast<float>(screenWidth) / 2.0f) - (cfg::dash_width / 2),
         .y = static_cast<float>(y),
         .width = cfg::dash_width,
         .height = cfg::dash_length},
        cfg::dash_color);
  }
}
int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  InitWindow(pong::config::screen_width, pong::config::screen_height,
             pong::config::window_title);

  SetTargetFPS(pong::config::fps);

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(pong::config::stage_background);

    drawCenterLine(GetScreenWidth(), GetScreenHeight());

    EndDrawing();
  }

  CloseWindow();
}