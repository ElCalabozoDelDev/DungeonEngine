#ifndef GAME_SYSTEMS_GRAYSCALE_FADE_SYSTEM_HPP
#define GAME_SYSTEMS_GRAYSCALE_FADE_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <game/state.hpp>

/// Fades world colour toward grayscale while paused / game-over — the curve
/// of learn-monogame-2d GameScene (`FADE_SPEED = 0.02` per 60 Hz frame),
/// expressed per second so it does not depend on the display's refresh rate.
class GrayscaleFadeSystem final : public de::System
{
public:
    void run(entt::registry& registry) override;

private:
    PlayState m_lastState = PlayState::Playing;
};

#endif // GAME_SYSTEMS_GRAYSCALE_FADE_SYSTEM_HPP
