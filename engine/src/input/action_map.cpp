#include <algorithm>
#include <engine/input/action_map.hpp>

namespace de
{
void ActionMap::bind(std::string_view action, SDL_Scancode key)
{
    auto& keys = m_bindings[std::string(action)];
    if (std::find(keys.begin(), keys.end(), key) == keys.end())
    {
        keys.push_back(key);
    }
}

void ActionMap::clear(std::string_view action)
{
    if (auto it = m_bindings.find(action); it != m_bindings.end())
    {
        it->second.clear();
    }
}

const std::vector<SDL_Scancode>* ActionMap::find(std::string_view action) const
{
    auto it = m_bindings.find(action);
    return it != m_bindings.end() ? &it->second : nullptr;
}

bool ActionMap::isDown(const InputState& input, std::string_view action) const
{
    const auto* keys = find(action);
    if (keys == nullptr)
    {
        return false;
    }
    return std::any_of(keys->begin(), keys->end(), [&input](SDL_Scancode key)
                       { return input.isDown(key); });
}

bool ActionMap::wasPressed(const InputState& input,
                           std::string_view action) const
{
    const auto* keys = find(action);
    if (keys == nullptr)
    {
        return false;
    }
    return std::any_of(keys->begin(), keys->end(), [&input](SDL_Scancode key)
                       { return input.wasPressed(key); });
}

bool ActionMap::wasReleased(const InputState& input,
                            std::string_view action) const
{
    const auto* keys = find(action);
    if (keys == nullptr)
    {
        return false;
    }
    return std::any_of(keys->begin(), keys->end(), [&input](SDL_Scancode key)
                       { return input.wasReleased(key); });
}

float ActionMap::axis(const InputState& input, std::string_view negative,
                      std::string_view positive) const
{
    const float value = (isDown(input, positive) ? 1.0f : 0.0f) -
                        (isDown(input, negative) ? 1.0f : 0.0f);
    return value;
}

} // namespace de
