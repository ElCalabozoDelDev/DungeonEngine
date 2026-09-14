#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <engine/spatial/quadtree.hpp>

/// Where the run is. This is the single source of truth: `de::Paused`, which
/// is what stops the loop stepping fixed systems, is derived from it and is
/// only ever written by setPlayState() (game/play_state.hpp). Write the state
/// through that function, never by assigning the field.
enum class PlayState
{
    Playing,
    Paused,
    GameOver
};

/// Run state shared by the gameplay systems and the HUD.
///
/// Installed once by GamePlugin and reset by InGameScene at the start of each
/// run; everything else reads it with `ctx().get`.
struct GameState
{
    int score = 0;
    PlayState playState = PlayState::Playing;
    /// The open floor in logical pixels: the map inset by one wall tile.
    de::Box<float> roomBounds{};
    int tileWidth = 20;
    int tileHeight = 20;
    int mapColumns = 16;
    int mapRows = 9;
};

struct AudioSettings
{
    int musicPercent = 60;
    int sfxPercent = 60;
};

#endif // GAME_STATE_HPP
