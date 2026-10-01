# Pong

Build simple a pong game in C++ using [raylib](https://www.raylib.com/). Requirements:

- Incorporate the basic rules of pong: if the ball touches a player's borders then a point is rewarded to the opponent. Ball is reset in the center and launched from the winning player. First to 5 wins. Reset the game scores and start ball in center with target towards player 1.
- Basic responsive UI: displays both player's scores in their respective corners, alongside side their names.
- 2-player control: player 1 moves with W/S, player 2 moves with UP_ARROW/DOWN_ARROW.
- CLI options: Supporting arguments: `-p1 [NAME]` and `-p2 [NAME]` so that each player can set their names.
    - (optional) Online multiplayer support. A process can host by invoking: `./[GAME_BIN] host -s [IP] -p [PORT]`; similarly, a client can connect: `./[GAME_BIN] client -s [IP] -p [PORT]`. Run `./[GAME_BIN]` to play local. If online multiplayer is in action, each process will default to using W/S for movement.

## Phases

1. **Scaffold & window**: CMake + `FetchContent` (raylib, Catch2), `pong_core` library, test target. Deliverable: resizable 60 FPS window with a center line, one passing test.
2. **Paddles & ball physics**: W/S and ↑/↓ movement, delta-time ball motion, wall/paddle bounces. Deliverable: two players can rally.
3. **Rules, scoring & UI**: scoring, serve/reset, first to 5, game-over state, names and scores in corners. Deliverable: complete local game.
4. **CLI arguments**: `-p1`/`-p2` parsed into a `Config`. Deliverable: custom player names.
5. **Networking prep** *(optional)*: separate input/simulation/render, fixed timestep, serializable game state. Deliverable: same gameplay, deterministic logic.
6. **Online play over TCP** *(optional)*: `host`/`client` modes, host-authoritative sync, RAII sockets, clean disconnects. Deliverable: networked match.

Each phase includes unit tests for its pure logic (math, rules, parsing, serialization).

