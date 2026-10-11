#include <pong/game.hpp>

namespace pong::game {
std::optional<paddle::Side> getScoringSide(entities::Ball ball,
                                           int screenWidth) {
    if (ball.pos.x + ball.radius < 0)
        return paddle::Side::Right;
    else if (ball.pos.x > screenWidth)
        return paddle::Side::Left;

    return std::nullopt;
}
} // namespace pong::game