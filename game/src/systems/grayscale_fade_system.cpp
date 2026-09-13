#include <algorithm>
#include <engine/core/delta_time.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <game/state.hpp>
#include <game/systems/grayscale_fade_system.hpp>

using namespace de;

namespace
{
/// Colour removed per second. DungeonSlime's GameScene.FADE_SPEED is 0.02 per
/// Update at a fixed 60 Hz; applying it per rendered frame made the fade 2.4x
/// faster on a 144 Hz display, so it is scaled by the frame delta instead.
constexpr float kFadePerSecond = 0.02f * 60.0f;
} // namespace

void GrayscaleFadeSystem::run(entt::registry& registry)
{
    const auto& state = registry.ctx().get<GameState>();
    auto& grade = registry.ctx().get<WorldColorGrade>();

    if (state.playState != m_lastState)
    {
        // Tutorial resets Saturation to 1.0f on entering pause / game over,
        // then fades on subsequent frames.
        if (state.playState == PlayState::Paused ||
            state.playState == PlayState::GameOver)
        {
            grade.colorAmount = 1.0f;
        }
        m_lastState = state.playState;
        return;
    }

    if (state.playState == PlayState::Playing)
    {
        grade.colorAmount = 1.0f;
        m_lastState = state.playState;
        return;
    }

    const float delta = registry.ctx().get<DeltaTime>().value;
    grade.colorAmount =
        std::max(0.0f, grade.colorAmount - kFadePerSecond * delta);
}
