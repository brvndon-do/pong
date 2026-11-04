#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <format>
#include <iostream>
#include <optional>
#include <pong/cli.hpp>
#include <pong/config.hpp>
#include <pong/entities.hpp>
#include <pong/game.hpp>
#include <pong/input.hpp>
#include <pong/network.hpp>
#include <random>
#include <raylib.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace cfg = pong::config;
namespace cli = pong::cli;
namespace entities = pong::entities;
namespace game = pong::game;
namespace input = pong::input;
namespace network = pong::network;

namespace {
enum class Environment : std::uint8_t { Local, Host, Client };

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

void drawGame(const game::GameContext& gameContext,
              std::string_view player1Name, std::string_view player2Name) {

    const entities::Paddle& player1{gameContext.player1};
    const entities::Paddle& player2{gameContext.player2};
    const entities::Ball& ball{gameContext.ball};
    const game::GameState state{gameContext.state};

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
        const std::string_view winningPlayer{
            gameContext.player1Score >= cfg::winning_score ? player1Name
                                                           : player2Name};
        const std::string winnerText =
            std::format("{} wins! {}", winningPlayer, cfg::gameover_msg);

        DrawText(winnerText.c_str(),
                 getCenterTextX(winnerText.c_str(), cfg::text_font_size,
                                cfg::screen_width),
                 cfg::screen_height / 2, cfg::text_font_size, cfg::text_color);
        break;
    }
    }

    // draw scoreboard
    const std::string player1Score{
        std::format("{}: {}", player1Name, gameContext.player1Score)};
    const std::string player2Score{
        std::format("{}: {}", player2Name, gameContext.player2Score)};

    DrawText(player1Score.c_str(), cfg::text_font_gap, cfg::text_font_gap,
             cfg::text_font_size, cfg::text_color);
    DrawText(player2Score.c_str(),
             cfg::screen_width -
                 MeasureText(player2Score.c_str(), cfg::text_font_size) -
                 cfg::text_font_gap,
             cfg::text_font_gap, cfg::text_font_size, cfg::text_color);

    EndDrawing();
}
} // namespace

int main(int argc, char* argv[]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-cfg::ball_max_serve_angle_rad,
                                               cfg::ball_max_serve_angle_rad);

    const auto parsedArgs{
        cli::parseArgs(std::vector<std::string_view>(argv + 1, argv + argc))};

    if (!parsedArgs.has_value()) {
        std::cerr << parsedArgs.error() << '\n';
        return EXIT_FAILURE;
    }

    std::optional<network::Listener> listener;
    std::optional<network::Connection> peer;
    std::string player1Name;
    std::string player2Name;

    const std::expected<Environment, std::string> setupResult{std::visit(
        [&](auto&& arg) -> std::expected<Environment, std::string> {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, cli::HostSettings>) {
                auto host{network::Listener::createHost(arg.endpoint)};
                if (!host.has_value())
                    return std::unexpected{host.error()};

                player1Name = arg.playerName;
                listener = std::move(host).value();

                return Environment::Host;
            } else if constexpr (std::is_same_v<T, cli::ClientSettings>) {
                auto connection{
                    network::Connection::createClient(arg.endpoint)};
                if (!connection.has_value())
                    return std::unexpected{connection.error()};

                player2Name = arg.playerName;
                peer = std::move(connection).value();

                return Environment::Client;
            } else if constexpr (std::is_same_v<T, cli::LocalSettings>) {
                player1Name = arg.player1Name;
                player2Name = arg.player2Name;

                return Environment::Local;
            } else {
                static_assert(false, "unhandled settings type");
            }
        },
        parsedArgs.value())};

    if (!setupResult.has_value()) {
        std::cerr << setupResult.error() << '\n';
        return EXIT_FAILURE;
    }

    const Environment env{setupResult.value()};

    // TODO: setup network play

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::screen_width, cfg::screen_height, cfg::window_title);
    SetTargetFPS(cfg::fps);

    game::GameContext gameContext{
        game::init(dist(gen), game::GameState::Serving)};
    float dtAccumulated{0.0f};
    bool servePending{false};
    bool restartPending{false};

    while (!WindowShouldClose()) {
        dtAccumulated += std::min(GetFrameTime(), cfg::dt_max);
        servePending |= IsKeyPressed(KEY_SPACE);
        restartPending |= IsKeyPressed(KEY_R);

        // fixed update
        while (dtAccumulated >= cfg::fixed_dt) {
            // TODO: move building input into a helper func
            input::InputState inputState{};

            switch (env) {
            case Environment::Host: {
                break;
            }
            case Environment::Client: {
                break;
            }
            case Environment::Local: {
                const std::int8_t player1Dir{getDirection(KEY_W, KEY_S)};
                const std::int8_t player2Dir{getDirection(KEY_UP, KEY_DOWN)};

                inputState = {
                    .player1 = input::PlayerInput{.direction = player1Dir,
                                                  .serve = servePending,
                                                  .restart = restartPending},
                    .player2 = input::PlayerInput{.direction = player2Dir,
                                                  .serve = servePending,
                                                  .restart = restartPending}};
                break;
            }
            }

            // client's state comes from host; do not update its gameContext
            if (env != Environment::Client)
                gameContext = game::update(gameContext, inputState,
                                           cfg::fixed_dt, dist(gen));

            // clear flags after first tick acknowledges the keypress
            servePending = false;
            restartPending = false;

            dtAccumulated -= cfg::fixed_dt;
        }

        drawGame(gameContext, player1Name, player2Name);
    }

    CloseWindow();
}