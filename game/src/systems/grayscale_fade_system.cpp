#include <algorithm>
#include <engine/graphics/world_color_grade.hpp>
#include <game/state.hpp>
#include <game/systems/grayscale_fade_system.hpp>

using namespace de;

namespace
{
/// Matches DungeonSlime GameScene.FADE_SPEED (per Update frame, not dt).
constexpr float kFadeSpeed = 0.02f;
} // namespace

void GrayscaleFadeSystem::run(entt::registry& registry)
{
    auto* state = registry.ctx().find<GameState>();
    if (state == nullptr)
    {
        return;
    }
    if (!registry.ctx().contains<WorldColorGrade>())
    {
        registry.ctx().emplace<WorldColorGrade>();
    }
    auto& grade = registry.ctx().get<WorldColorGrade>();

    if (state->playState != m_lastState)
    {
        // Tutorial resets Saturation to 1.0f on entering pause / game over,
        // then fades on subsequent frames.
        if (state->playState == PlayState::Paused ||
            state->playState == PlayState::GameOver)
        {
            grade.colorAmount = 1.0f;
        }
        m_lastState = state->playState;
        return;
    }

    if (state->playState == PlayState::Playing)
    {
        grade.colorAmount = 1.0f;
        return;
    }

    grade.colorAmount = std::max(0.0f, grade.colorAmount - kFadeSpeed);
}
