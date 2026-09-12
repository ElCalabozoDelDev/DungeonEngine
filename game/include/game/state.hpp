#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

/// Run state shared by the gameplay systems and the HUD, held in the registry
/// context.
struct GameState
{
    int itemsCollected = 0;
    int itemsTotal = 0;
    /// Set when the player runs out of health; the flow handler turns it into
    /// a scene change, so the transition happens in one place.
    bool gameOver = false;
};

#endif // GAME_STATE_HPP
