#ifndef GAME_RUN_RUN_CONFIG_HPP
#define GAME_RUN_RUN_CONFIG_HPP

#include <cstdint>

/// Tunables for a full run. Defaults are playtest starting points — not locked
/// design — and may be overridden from CLI / future config without an ADR.
struct RunConfig
{
    /// Fixed floors per run (N). Configurable for balancing.
    int floorsPerRun = 3;
    /// Inclusive minimum floor index (1-based) where the objective may spawn.
    int objectiveFloor = 3;
    /// libtcod FOV radius in tiles.
    int fovRadiusTiles = 8;
    /// Deterministic generator seed. Same seed → same floors.
    std::uint32_t seed = 1;
    /// Tile size in pixels (matches the dungeon tileset).
    float tileSize = 16.0f;
    int mapColumns = 40;
    int mapRows = 30;
};

#endif // GAME_RUN_RUN_CONFIG_HPP
