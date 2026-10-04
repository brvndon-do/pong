#include <catch2/catch_test_macros.hpp>
#include <pong/config.hpp>

TEST_CASE("Configuration Test", "[config]") {
  STATIC_REQUIRE(pong::config::fps == 60);
}