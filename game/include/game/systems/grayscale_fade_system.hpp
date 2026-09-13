#ifndef GAME_SYSTEMS_GRAYSCALE_FADE_SYSTEM_HPP
#define GAME_SYSTEMS_GRAYSCALE_FADE_SYSTEM_HPP

#include <engine/systems/system.hpp>
#include <game/state.hpp>

/// Fades world colour toward grayscale while paused / game-over — same curve
/// as learn-monogame-2d GameScene (`FADE_SPEED = 0.02` per frame).
class GrayscaleFadeSystem final : public de::System
{
public:
    void run(entt::registry& registry) override;

private:
    PlayState m_lastState = PlayState::Playing;
};

#endif // GAME_SYSTEMS_GRAYSCALE_FADE_SYSTEM_HPP
