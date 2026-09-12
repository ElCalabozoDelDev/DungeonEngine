#ifndef GAME_PROC_FLOOR_BLUEPRINT_HPP
#define GAME_PROC_FLOOR_BLUEPRINT_HPP

#include <cstdint>
#include <vector>

/// One generated floor before it becomes ECS entities.
struct FloorBlueprint
{
    enum class Cell : std::uint8_t
    {
        Void = 0,
        Floor = 1,
        Wall = 2,
    };

    struct Marker
    {
        enum class Kind : std::uint8_t
        {
            Entrance,
            StairsDown,
            StairsUp,
            Enemy,
            ScoreItem,
            Objective,
        };

        Kind kind = Kind::Entrance;
        int column = 0;
        int row = 0;
    };

    int columns = 0;
    int rows = 0;
    float tileSize = 16.0f;
    int floorIndex = 1;
    std::vector<Cell> cells;
    std::vector<Marker> markers;

    bool inBounds(int column, int row) const
    {
        return column >= 0 && row >= 0 && column < columns && row < rows;
    }

    Cell at(int column, int row) const
    {
        if (!inBounds(column, row))
        {
            return Cell::Void;
        }
        return cells[static_cast<std::size_t>(row * columns + column)];
    }

    void set(int column, int row, Cell cell)
    {
        if (!inBounds(column, row))
        {
            return;
        }
        cells[static_cast<std::size_t>(row * columns + column)] = cell;
    }
};

#endif // GAME_PROC_FLOOR_BLUEPRINT_HPP
