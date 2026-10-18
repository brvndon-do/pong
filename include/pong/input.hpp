#pragma once

#include <cstdint>
#include <type_traits>

namespace pong::input {
struct PlayerInput {
    std::int8_t direction{0};
    bool serve;
    bool restart;
};

struct InputState {
    PlayerInput player1{};
    PlayerInput player2{};
};

static_assert(std::is_trivially_copyable_v<InputState>);

} // namespace pong::input