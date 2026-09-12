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
    /// Set when the player returns to the run entrance with the objective.
    bool victory = false;

    int currentFloor = 1;
    bool hasObjective = false;
    /// Spawn tile of floor 1 for the run; win check uses this cell.
    int entranceColumn = 0;
    int entranceRow = 0;
    bool entranceRecorded = false;

    /// Requested by stairs overlap; InGameScene / progression applies it.
    int pendingFloorChange = 0;
};

#endif // GAME_STATE_HPP
