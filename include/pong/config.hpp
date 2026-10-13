#pragma once

#include <numbers>
#include <raylib.h>

namespace pong::config {
inline constexpr const char* window_title{"Pong"};
inline constexpr int screen_width{1280};
inline constexpr int screen_height{720};
inline constexpr int fps{60};

inline constexpr float paddle_width{15.0f};
inline constexpr float paddle_length{150.0f};
inline constexpr float paddle_speed{400.0f}; // paddle_speed_px_per_s
inline constexpr Color paddle_color{WHITE};

inline constexpr float ball_radius{15.0f};
inline constexpr float ball_speed{450.0f};
inline constexpr float ball_max_serve_angle_deg{45.0f};
inline constexpr float ball_max_serve_angle_rad{
    ball_max_serve_angle_deg * std::numbers::pi_v<float> / 180.0f};
inline constexpr Color ball_color{ORANGE};

inline constexpr float dash_width{15.0f};
inline constexpr float dash_length{30.0f};
inline constexpr float dash_gap{60.0f};
inline constexpr Color dash_color{WHITE};

inline constexpr int text_font_gap{2};
inline constexpr int text_font_size{36};
inline constexpr Color text_color{WHITE};

inline constexpr int winning_score{5};
inline constexpr Color stage_background{BLACK};
} // namespace pong::config