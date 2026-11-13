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
#include <pong/network.hpp>
#include <pong/network_protocol.hpp>
#include <pong/session.hpp>
#include <random>
#include <raylib.h>
#include <string>
#include <string_view>
#include <vector>

namespace cfg = pong::config;
namespace cli = pong::cli;
namespace entities = pong::entities;
namespace game = pong::game;
namespace input = pong::input;
namespace session = pong::session;
namespace network = pong::network;
namespace protocol = pong::network::protocol;

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

[[nodiscard]] std::expected<void, std::string>
handleHostUpdate(network::Connection& connection,
                 input::InputState& latestInput,
                 const game::GameContext& context) {

    // update player2 input
    if (auto res{connection.receive()}; !res.has_value())
        return std::unexpected{res.error()};

    while (true) {
        auto expectedMessage{connection.nextMessage()};
        if (!expectedMessage)
            return std::unexpected{expectedMessage.error()};

        if (!*expectedMessage)
            break;

        const auto& message{expectedMessage->value()};
        if (const protocol::InputPayload* expectedInputPayload{
                std::get_if<protocol::InputPayload>(&message)}) {
            latestInput.player2.direction = expectedInputPayload->direction;
            latestInput.player2.restart |= expectedInputPayload->restart;
            latestInput.player2.serve |= expectedInputPayload->serve;
        }
    }

    // send snapshot payload after update
    protocol::SnapshotPayload snapshotPayload{
        .player1PaddleY = context.player1.rect.y,
        .player2PaddleY = context.player2.rect.y,
        .ballX = context.ball.pos.x,
        .ballY = context.ball.pos.y,
        .gameState = static_cast<std::uint8_t>(context.state),
        .player1Score = static_cast<std::uint8_t>(context.player1Score),
        .player2Score = static_cast<std::uint8_t>(context.player2Score)};

    auto encodedPayload{protocol::encodeSnapshotPayload(snapshotPayload)};
    if (auto res{connection.send(encodedPayload)}; !res.has_value())
        return std::unexpected{res.error()};

    return {};
}

[[nodiscard]] std::expected<void, std::string>
handleClientUpdate(network::Connection& connection,
                   const input::InputState& latestInput,
                   game::GameContext& context) {

    // update client's game context
    if (auto res{connection.receive()}; !res.has_value())
        return std::unexpected{res.error()};

    while (true) {
        auto expectedMessage{connection.nextMessage()};
        if (!expectedMessage)
            return std::unexpected{expectedMessage.error()};

        if (!*expectedMessage)
            break;

        const auto& message{expectedMessage->value()};
        if (const protocol::SnapshotPayload* expectedSnapshotPayload{
                std::get_if<protocol::SnapshotPayload>(&message)}) {
            context.player1.rect.y = expectedSnapshotPayload->player1PaddleY;
            context.player2.rect.y = expectedSnapshotPayload->player2PaddleY;
            context.ball.pos.x = expectedSnapshotPayload->ballX;
            context.ball.pos.y = expectedSnapshotPayload->ballY;
            context.state = static_cast<game::GameState>(
                expectedSnapshotPayload->gameState);
            context.player1Score = expectedSnapshotPayload->player1Score;
            context.player2Score = expectedSnapshotPayload->player2Score;
        }
    }

    // send input payload after update
    protocol::InputPayload inputPayload{.direction =
                                            latestInput.player2.direction,
                                        .serve = latestInput.player2.serve,
                                        .restart = latestInput.player2.restart};

    auto encodedPayload{protocol::encodeInputPayload(inputPayload)};
    if (auto res{connection.send(encodedPayload)}; !res.has_value())
        return std::unexpected{res.error()};

    return {};
}
} // namespace

int main(int argc, char* argv[]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-cfg::ball_max_serve_angle_rad,
                                               cfg::ball_max_serve_angle_rad);

    auto parsedArgs{
        cli::parseArgs(std::vector<std::string_view>(argv + 1, argv + argc))};
    if (!parsedArgs.has_value()) {
        std::cerr << parsedArgs.error() << '\n';
        return EXIT_FAILURE;
    }

    auto session{session::createSession(parsedArgs.value())};
    if (!session.has_value()) {
        std::cerr << session.error() << '\n';
        return EXIT_FAILURE;
    }

    // window + game loop
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::screen_width, cfg::screen_height, cfg::window_title);
    SetTargetFPS(cfg::fps);

    game::GameContext gameContext{
        game::init(dist(gen), game::GameState::Serving)};
    float dtAccumulated{0.0f};
    bool servePending{false};
    bool restartPending{false};
    input::InputState inputState{};
    std::optional<std::string> networkError;

    // TODO: properly handle networkErrors instead of closing the game loop
    while (!WindowShouldClose() && !networkError) {
        dtAccumulated += std::min(GetFrameTime(), cfg::dt_max);
        servePending |= IsKeyPressed(KEY_SPACE);
        restartPending |= IsKeyPressed(KEY_R);

        // TODO: properly handle networkErrors instead of closing the game loop
        // fixed update
        while (dtAccumulated >= cfg::fixed_dt && !networkError) {
            switch (session->env) {
            case session::Environment::Host: {
                inputState.player1.direction = getDirection(KEY_W, KEY_S);
                inputState.player1.serve = servePending;
                inputState.player1.restart = restartPending;

                if (auto res{handleHostUpdate(session->peer.value(), inputState,
                                              gameContext)};
                    !res.has_value())
                    networkError = res.error();

                break;
            }
            case session::Environment::Client: {
                inputState.player2.direction = getDirection(KEY_UP, KEY_DOWN);
                inputState.player2.serve = servePending;
                inputState.player2.restart = restartPending;

                if (auto res{handleClientUpdate(session->peer.value(),
                                                inputState, gameContext)};
                    !res.has_value())
                    networkError = res.error();

                break;
            }
            case session::Environment::Local: {
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
            if (session->env != session::Environment::Client)
                gameContext = game::update(gameContext, inputState,
                                           cfg::fixed_dt, dist(gen));

            // clear flags after first tick acknowledges the keypress
            servePending = false;
            restartPending = false;

            // this should be okay since in Local mode, inputState is rebuilt on
            // every fixed frame update
            inputState.player2.serve = false;
            inputState.player2.restart = false;

            dtAccumulated -= cfg::fixed_dt;
        }

        drawGame(gameContext, session->player1Name, session->player2Name);
    }

    if (networkError)
        std::cerr << std::format("network error: {}\n", *networkError);

    CloseWindow();
}