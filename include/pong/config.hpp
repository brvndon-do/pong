#pragma once

#include <raylib.h>

namespace pong::config {
inline constexpr const char *window_title = "Pong";
inline constexpr int screen_width = 1280;
inline constexpr int screen_height = 720;
inline constexpr int fps = 60;

// TODO: refine these if necessary
inline constexpr float paddle_width = 15.0f;
inline constexpr float paddle_length = 60.0f;
inline constexpr float paddle_speed = 400.0f; // paddle_speed_px_per_s

inline constexpr float dash_width = 15.0f;
inline constexpr float dash_length = 30.0f;
inline constexpr float dash_gap = 60.0f;
inline constexpr Color dash_color = WHITE;

inline constexpr Color paddle_color = WHITE;
inline constexpr Color stage_background = BLACK;
} // namespace pong::config