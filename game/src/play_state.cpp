#include <engine/core/paused.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <game/play_state.hpp>
#include <game/state.hpp>

using namespace de;

void setPlayState(entt::registry& registry, PlayState next)
{
    if (!registry.ctx().contains<GameState>())
    {
        registry.ctx().emplace<GameState>();
    }
    registry.ctx().get<GameState>().playState = next;
    if (auto* paused = registry.ctx().find<Paused>(); paused != nullptr)
    {
        paused->value = (next != PlayState::Playing);
    }
}

void resetRunPresentation(entt::registry& registry)
{
    setPlayState(registry, PlayState::Playing);
    if (auto* grade = registry.ctx().find<WorldColorGrade>();
        grade != nullptr)
    {
        grade->colorAmount = 1.0f;
    }
}
