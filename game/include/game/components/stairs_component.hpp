#ifndef GAME_COMPONENTS_STAIRS_COMPONENT_HPP
#define GAME_COMPONENTS_STAIRS_COMPONENT_HPP

enum class StairsDirection
{
    Down,
    Up,
};

/// Overlap target that requests a floor change (handled by ProgressionSystem).
struct StairsComponent
{
    StairsDirection direction = StairsDirection::Down;
};

#endif // GAME_COMPONENTS_STAIRS_COMPONENT_HPP
