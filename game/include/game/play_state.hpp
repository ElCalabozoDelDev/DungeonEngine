#ifndef GAME_PLAY_STATE_HPP
#define GAME_PLAY_STATE_HPP

#include <entt/entt.hpp>
#include <game/state.hpp>

/// Writes `GameState::playState` and keeps `de::Paused` in sync (`Paused`
/// when not Playing). The only writer of either: that is what keeps them from
/// disagreeing. Requires GameState and Paused in the context (GamePlugin
/// installs both).
void setPlayState(entt::registry& registry, PlayState next);

/// Clear pause/game-over presentation when leaving the run (Title/Options).
/// Also requires WorldColorGrade.
void resetRunPresentation(entt::registry& registry);

#endif // GAME_PLAY_STATE_HPP
