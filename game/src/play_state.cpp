#include <engine/core/paused.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <game/play_state.hpp>
#include <game/state.hpp>

using namespace de;

void setPlayState(entt::registry& registry, PlayState next)
{
    registry.ctx().get<GameState>().playState = next;
    registry.ctx().get<Paused>().value = (next != PlayState::Playing);
}

void resetRunPresentation(entt::registry& registry)
{
    setPlayState(registry, PlayState::Playing);
    registry.ctx().get<WorldColorGrade>().colorAmount = 1.0f;
}
