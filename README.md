# Pong

Build simple a pong game in C++ using [raylib](https://www.raylib.com/). Requirements:

- Incorporate the basic rules of pong: if the ball touches a player's borders then a point is rewarded to the opponent. Ball is reset in the center and launched from the winning player. First to 5 wins. Reset the game scores and start ball in center with target towards player 1.
- Basic responsive UI: displays both player's scores in their respective corners, alongside side their names.
- 2-player control: player 1 moves with W/S, player 2 moves with UP_ARROW/DOWN_ARROW.
- CLI options: Supporting arguments: `-n1 [NAME]` and `-n2 [NAME]` so that each player can set their names.
    - (optional) Online multiplayer support. A process can host by invoking: `./[GAME_BIN] host -s [IP] -p [PORT] -n [NAME]`; similarly, a client can connect: `./[GAME_BIN] client -s [IP] -p [PORT] -n [NAME]`. Run `./[GAME_BIN]` to play local. If online multiplayer is in action, each process will default to using W/S for movement.

## Phases

1. **Scaffold & window**: CMake + `FetchContent` (raylib, Catch2), `pong_core` library, test target. Deliverable: resizable 60 FPS window with a center line, one passing test.
2. **Paddles & ball physics**: W/S and ↑/↓ movement, delta-time ball motion, wall/paddle bounces. Deliverable: two players can rally.
3. **Rules, scoring & UI**: scoring, serve/reset, first to 5, game-over state, names and scores in corners. Deliverable: complete local game.
4. **CLI arguments**: `-n1`/`-n2` parsed into a `Config`. Deliverable: custom player names.
5. **Networking prep** *(optional)*: separate input/simulation/render, fixed timestep, serializable game state. Deliverable: same gameplay, deterministic logic.
6. **Online play over TCP** *(optional)*: `host`/`client` modes, host-authoritative sync, RAII sockets, clean disconnects. Deliverable: networked match.

## Takeaways and High-level Learning

- `std::expected`, `std::optional`, `std::span`, and `std::variant` provided graceful options on handling certain cases:
    - `std::expected`/`std::optional` allowed me to write safe code that can error or be invalid.
    - `std::span` gave me non-owning views, which is great since ownership still remains to the caller. It also allows one parameter type that can accept arrays, veectors, or slices without copying.
    - `std::variant` allowed me to use `std::visit` to perform a "pattern match" type of the underlying object.
- Templates/Concepts used in [src/cli.cpp](src/cli.cpp), although extremely basic, gave me a taste of how I can work with a `struct` (shape) that were similar. By enforcing `std::same_as`, the caller is required to pass `HostSettings` or `ClientSettings`.
- RAII and proper class setup to establish the rule of five and move semantics.
    - Intentional deletion of the copy constructor/assignment operator since we do not want to support copying the `Socket` object.
    - Using the `friend` allowed `Connection` to be exposed to `Listener`'s members; this might be a bad usage of `friend` though, resulting in poor encapsulation logic...
    - Transferring ownership of the `Socket` object.
- Prefix `::` to explicitly access identifiers in the global namespace.
- Namespaces to organize related functions and to prevent naming conflicts. Additionally, anonymous namespaces.
    - `namespace` keyword can be used to provide "aliases" to longer-typed namespaces.
- Catch2 testing library exposure, which I'm still not quite knowledgeable to form a solid opinion...
- `inline constexpr` to define compile-time constants without violating ODR (one definition rule).
- `if-init` pattern, check expected/optional pattern, and other "modern" patterns for handling cases.
- Network code for server/client interactions, which conceptually sounds really simple, but I had to be careful and pay extra mind to encoding/decoding the bytes to send.
    - Bitwise and bitshift operators were used. Bitwise shifts work on the value, not memory layout. Bitshift to ensure a consistent endianness and bitwise (`|=`) to rebuild the correct bits.
    - TCP is a byte stream, NOT a stream of messages. That's why a receive buffer and the `nextMessage` framing was needed.
    - Non-blocking sockets (`EAGAIN`), `TCP_NODELAY` and Nagle's algorithm, and `MSG_NOSIGNAL` for partial writes.