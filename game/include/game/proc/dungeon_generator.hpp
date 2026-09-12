#ifndef GAME_PROC_DUNGEON_GENERATOR_HPP
#define GAME_PROC_DUNGEON_GENERATOR_HPP

#include <game/proc/floor_blueprint.hpp>
#include <game/run/run_config.hpp>

/// Room-and-corridor dungeon floor generator.
///
/// Deterministic for a given (seed, floorIndex, config size). Prefer readable
/// rooms over maze noise — see SYS-PROC.
class DungeonGenerator
{
public:
    FloorBlueprint generate(const RunConfig& config, int floorIndex) const;
};

#endif // GAME_PROC_DUNGEON_GENERATOR_HPP
