#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include <engine/spatial/quadtree.hpp>

enum class PlayState
{
    Playing,
    Paused,
    GameOver
};

/// Axis-aligned room in logical pixels (inset from the tilemap walls).
struct Rectangle
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    float left() const { return x; }
    float top() const { return y; }
    float right() const { return x + w; }
    float bottom() const { return y + h; }

    de::Box<float> asBox() const { return de::Box<float>(x, y, w, h); }
};

/// Run state shared by the gameplay systems and the HUD.
struct GameState
{
    int score = 0;
    PlayState playState = PlayState::Playing;
    /// Legacy alias: true when playState == GameOver.
    bool gameOver = false;
    Rectangle roomBounds{};
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
