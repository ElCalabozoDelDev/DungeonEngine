#include <SDL.h>
#include <algorithm>
#include <doctest/doctest.h>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <vector>

using namespace de;

namespace
{
/// Builds an InputState with `down` held this frame and `previouslyDown` held
/// the frame before, so edge detection can be exercised.
InputState stateWith(const std::vector<SDL_Scancode>& down,
                     const std::vector<SDL_Scancode>& previouslyDown = {})
{
    InputState input;
    std::vector<Uint8> keys(SDL_NUM_SCANCODES, 0);

    for (auto key : previouslyDown)
    {
        keys[key] = 1;
    }
    input.setKeyboard(keys.data(), SDL_NUM_SCANCODES);
    input.beginFrame(); // what was set above becomes "previous"

    std::fill(keys.begin(), keys.end(), Uint8{0});
    for (auto key : down)
    {
        keys[key] = 1;
    }
    input.setKeyboard(keys.data(), SDL_NUM_SCANCODES);
    return input;
}

} // namespace

TEST_CASE("an action responds to any of its bindings")
{
    ActionMap actions;
    actions.bind("move_up", SDL_SCANCODE_UP);
    actions.bind("move_up", SDL_SCANCODE_W);

    CHECK(actions.isDown(stateWith({SDL_SCANCODE_UP}), "move_up"));
    CHECK(actions.isDown(stateWith({SDL_SCANCODE_W}), "move_up"));
    CHECK_FALSE(actions.isDown(stateWith({SDL_SCANCODE_S}), "move_up"));
}

TEST_CASE("rebinding an action")
{
    ActionMap actions;
    actions.bind("jump", SDL_SCANCODE_SPACE);
    REQUIRE(actions.isDown(stateWith({SDL_SCANCODE_SPACE}), "jump"));

    actions.clear("jump");
    actions.bind("jump", SDL_SCANCODE_Z);

    CHECK_FALSE(actions.isDown(stateWith({SDL_SCANCODE_SPACE}), "jump"));
    CHECK(actions.isDown(stateWith({SDL_SCANCODE_Z}), "jump"));
}

TEST_CASE("an unbound action is never down")
{
    ActionMap actions;
    CHECK_FALSE(actions.isDown(stateWith({SDL_SCANCODE_A}), "nothing"));
}

TEST_CASE("edge detection separates pressed, held and released")
{
    ActionMap actions;
    actions.bind("fire", SDL_SCANCODE_F);

    const InputState justPressed = stateWith({SDL_SCANCODE_F}, {});
    const InputState held = stateWith({SDL_SCANCODE_F}, {SDL_SCANCODE_F});
    const InputState released = stateWith({}, {SDL_SCANCODE_F});

    CHECK(actions.wasPressed(justPressed, "fire"));
    CHECK_FALSE(actions.wasPressed(held, "fire"));
    CHECK(actions.isDown(held, "fire"));
    CHECK(actions.wasReleased(released, "fire"));
    CHECK_FALSE(actions.wasReleased(held, "fire"));
}

TEST_CASE("axis from a pair of opposing actions")
{
    ActionMap actions;
    actions.bind("left", SDL_SCANCODE_LEFT);
    actions.bind("right", SDL_SCANCODE_RIGHT);

    CHECK(actions.axis(stateWith({SDL_SCANCODE_RIGHT}), "left", "right") ==
          doctest::Approx(1.0f));
    CHECK(actions.axis(stateWith({SDL_SCANCODE_LEFT}), "left", "right") ==
          doctest::Approx(-1.0f));
    CHECK(actions.axis(stateWith({}), "left", "right") ==
          doctest::Approx(0.0f));
    CHECK(actions.axis(stateWith({SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT}),
                       "left", "right") == doctest::Approx(0.0f));
}

TEST_CASE("input is ignored while the GUI has the keyboard")
{
    // Typing into an inspector field must not also drive the player.
    ActionMap actions;
    actions.bind("move_up", SDL_SCANCODE_W);

    InputState input = stateWith({SDL_SCANCODE_W});
    REQUIRE(actions.isDown(input, "move_up"));

    input.setCaptured(true, false);
    CHECK_FALSE(actions.isDown(input, "move_up"));
}
