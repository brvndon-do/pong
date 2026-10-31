#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <format>
#include <iostream>
#include <pong/cli.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/game.hpp>
#include <pong/input.hpp>
#include <random>
#include <raylib.h>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace cfg = pong::config;
namespace cli = pong::cli;
namespace entities = pong::entities;
namespace game = pong::game;
namespace input = pong::input;

namespace {
void drawCenterLine() {
    for (float y = 0; y < cfg::screen_height;
         y += (cfg::dash_gap + cfg::dash_length)) {
        DrawRectangleRec({.x = (static_cast<float>(cfg::screen_width) / 2.0f) -
                               (cfg::dash_width / 2.0f),
                          .y = y,
                          .width = cfg::dash_width,
                          .height = cfg::dash_length},
                         cfg::dash_color);
    }
}

std::int8_t getDirection(KeyboardKey up, KeyboardKey down) {
    return static_cast<std::int8_t>(IsKeyDown(down) - IsKeyDown(up));
}

int getCenterTextX(const char* msg, int fontSize, int screenWidth) {
    return (screenWidth / 2) - MeasureText(msg, fontSize) / 2;
}
} // namespace

int main(int argc, char* argv[]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-cfg::ball_max_serve_angle_rad,
                                               cfg::ball_max_serve_angle_rad);

    const auto settings{
        cli::parseArgs(std::vector<std::string_view>(argv + 1, argv + argc))};

    if (!settings.has_value()) {
        std::cerr << settings.error() << '\n';
        return EXIT_FAILURE;
    }

    // TODO: support network play; for now, just extract local play settings
    if (!std::holds_alternative<cli::LocalSettings>(settings.value())) {
        std::cerr << "Network play not supported!\n";
        return EXIT_FAILURE;
    }
    cli::LocalSettings userSettings{
        std::get<cli::LocalSettings>(settings.value())};

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::screen_width, cfg::screen_height, cfg::window_title);
    SetTargetFPS(cfg::fps);

    game::GameContext gameContext{
        game::init(dist(gen), game::GameState::Serving)};
    entities::Paddle& player1{gameContext.player1};
    entities::Paddle& player2{gameContext.player2};
    entities::Ball& ball{gameContext.ball};
    game::GameState& state{gameContext.state};
    float dtAccumulated{0.0f};
    bool servePending{false};
    bool restartPending{false};

    while (!WindowShouldClose()) {
        dtAccumulated += std::min(GetFrameTime(), cfg::dt_max);

        const std::int8_t player1Dir{getDirection(KEY_W, KEY_S)};
        const std::int8_t player2Dir{getDirection(KEY_UP, KEY_DOWN)};

        servePending |= IsKeyPressed(KEY_SPACE);
        restartPending |= IsKeyPressed(KEY_R);

        while (dtAccumulated >= cfg::fixed_dt) {
            const input::InputState inputState{
                .player1 = input::PlayerInput{.direction = player1Dir,
                                              .serve = servePending,
                                              .restart = restartPending},
                .player2 = input::PlayerInput{.direction = player2Dir,
                                              .serve = servePending,
                                              .restart = restartPending}};

            gameContext =
                game::update(gameContext, inputState, cfg::fixed_dt, dist(gen));

            // clear flags after first tick acknowledges the keypress
            servePending = false;
            restartPending = false;

            dtAccumulated -= cfg::fixed_dt;
        }

        // draw
        BeginDrawing();

        ClearBackground(cfg::stage_background);

        drawCenterLine();

        // draw entities
        DrawRectangleRec(player1.rect, cfg::paddle_color);
        DrawRectangleRec(player2.rect, cfg::paddle_color);

        switch (state) {
        case game::GameState::Playing:
            // draw only when state is playing
            DrawCircleV(ball.pos, ball.radius, cfg::ball_color);
            break;
        case game::GameState::Serving:
            DrawText(cfg::start_msg,
                     getCenterTextX(cfg::start_msg, cfg::text_font_size,
                                    cfg::screen_width),
                     (cfg::screen_height / 2), cfg::text_font_size,
                     cfg::text_color);
            break;
        case game::GameState::GameOver: {
            const std::string winningPlayer =
                gameContext.player1Score >= cfg::winning_score
                    ? userSettings.player1Name
                    : userSettings.player2Name;
            const std::string winnerText =
                std::format("{} wins! {}", winningPlayer, cfg::gameover_msg);

            DrawText(winnerText.c_str(),
                     getCenterTextX(winnerText.c_str(), cfg::text_font_size,
                                    cfg::screen_width),
                     cfg::screen_height / 2, cfg::text_font_size,
                     cfg::text_color);
            break;
        }
        }

        // draw scoreboard
        const std::string player1Score{std::format(
            "{}: {}", userSettings.player1Name, gameContext.player1Score)};
        const std::string player2Score{std::format(
            "{}: {}", userSettings.player2Name, gameContext.player2Score)};

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