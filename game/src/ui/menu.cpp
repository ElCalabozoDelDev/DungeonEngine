#include <algorithm>
#include <engine/core/delta_time.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <game/ui/menu.hpp>
#include <game/ui/ui_layout.hpp>
#include <game/ui/ui_skin.hpp>

using namespace de;

namespace game::ui
{
void MenuNav::step(int delta)
{
    if (m_count <= 0)
    {
        return;
    }
    const int next = m_focus + delta;
    if (m_ends == Ends::Wrap)
    {
        m_focus = (next % m_count + m_count) % m_count;
    }
    else
    {
        m_focus = std::clamp(next, 0, m_count - 1);
    }
}

void MenuNav::update(entt::registry& registry,
                     std::initializer_list<std::string_view> back,
                     std::initializer_list<std::string_view> forward)
{
    const auto any = [&registry](std::initializer_list<std::string_view> keys)
    {
        for (auto action : keys)
        {
            if (pressed(registry, action))
            {
                return true;
            }
        }
        return false;
    };

    if (any(back))
    {
        step(-1);
        playUi(registry);
    }
    if (any(forward))
    {
        step(+1);
        playUi(registry);
    }
}

bool pressed(entt::registry& registry, std::string_view action)
{
    const auto* input = registry.ctx().find<InputState>();
    const auto* actions = registry.ctx().find<ActionMap>();
    return input != nullptr && actions != nullptr &&
           actions->wasPressedRaw(*input, action);
}

double uiClock(entt::registry& registry)
{
    const auto* dt = registry.ctx().find<DeltaTime>();
    return dt != nullptr ? dt->elapsed : 0.0;
}

bool menuButton(entt::registry& registry, ImDrawList* draw, const char* id,
                std::string_view label, ImVec2 topLeft, bool focused,
                bool confirm)
{
    const ImVec2 size = buttonSizeForLabel(registry, label);
    const ImVec2 bottomRight(topLeft.x + size.x, topLeft.y + size.y);
    drawButton(registry, draw, topLeft, size, focused, uiClock(registry));
    drawCenteredText(registry, draw, topLeft, bottomRight, label, kFontButton,
                     kButtonTextColor);
    return hitButton(id, topLeft, size) || (confirm && focused);
}

} // namespace game::ui
