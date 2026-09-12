#ifndef DE_INPUT_ACTION_MAP_HPP
#define DE_INPUT_ACTION_MAP_HPP

#include <SDL.h>
#include <engine/input/input_state.hpp>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace de
{
/// Named actions bound to keys, published in the registry context.
///
/// Gameplay asks for "move_up" rather than SDL_SCANCODE_UP, so controls can be
/// rebound without touching a system, and one action can have several bindings
/// (arrows and WASD both driving the same movement).
///
/// Actions are strings so a game can name its own without the engine knowing
/// them. The cost is a map lookup per query, which is a handful per frame --
/// this is not a hot path.
class ActionMap
{
public:
    /// Adds a binding. Calling it again for the same action adds an
    /// alternative rather than replacing the first.
    void bind(std::string_view action, SDL_Scancode key);

    /// Removes every binding for an action.
    void clear(std::string_view action);

    bool isDown(const InputState& input, std::string_view action) const;
    bool wasPressed(const InputState& input, std::string_view action) const;
    bool wasReleased(const InputState& input, std::string_view action) const;

    /// -1, 0 or +1 from a pair of opposing actions, for building a direction.
    float axis(const InputState& input, std::string_view negative,
               std::string_view positive) const;

private:
    const std::vector<SDL_Scancode>* find(std::string_view action) const;

    std::map<std::string, std::vector<SDL_Scancode>, std::less<>> m_bindings;
};

} // namespace de

#endif // DE_INPUT_ACTION_MAP_HPP
