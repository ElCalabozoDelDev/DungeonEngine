#ifndef GAME_SIM_NAV_GRID_HPP
#define GAME_SIM_NAV_GRID_HPP

#include <engine/core/vector_2d.hpp>
#include <entt/entt.hpp>
#include <vector>

/// One cell of the navigation grid. Out-of-range cells use column -1.
struct NavCell
{
    int column = -1;
    int row = -1;

    friend bool operator==(const NavCell& a, const NavCell& b)
    {
        return a.column == b.column && a.row == b.row;
    }
};

class NavGrid;

/// The result of one breadth-first sweep: distance in cells from the origin,
/// and enough information to walk a path back towards it.
///
/// Held separately from the grid because the grid is static for a level and a
/// field is recomputed as the player moves.
class NavField
{
public:
    bool reachable(NavCell cell) const;

    /// Cells from the origin, or -1 when unreachable.
    int distance(NavCell cell) const;

    /// The first cell to step to when heading for `goal`. Returns the origin
    /// itself when the goal is the origin or is unreachable.
    NavCell stepTowards(NavCell goal) const;

    NavCell origin() const { return m_origin; }

private:
    friend class NavGrid;

    int index(NavCell cell) const;

    int m_columns = 0;
    int m_rows = 0;
    NavCell m_origin;
    std::vector<int> m_distance; // -1 when unvisited
    std::vector<int> m_previous; // index of the cell stepped from, or -1
};

/// Static occupancy of a level, in tiles, with breadth-first search over it.
///
/// Built once per level from the tile entities of the solid layers. Enemies
/// are deliberately not in it: they are not solid to anything (only the
/// Collision and Overlay tile layers are), so the grid never changes while a
/// level is running.
class NavGrid
{
public:
    /// Reads the solid tile layers out of the registry.
    void build(entt::registry& registry);

    /// Empty grid of a given size, for tests.
    NavGrid& reset(int columns, int rows, float tileSize);
    NavGrid& setSolid(int column, int row, bool solid);

    bool empty() const { return m_columns == 0 || m_rows == 0; }
    int columns() const { return m_columns; }
    int rows() const { return m_rows; }
    float tileSize() const { return m_tileSize; }

    bool inBounds(NavCell cell) const;

    /// Out-of-bounds counts as solid, so callers never walk off the map.
    bool isSolid(NavCell cell) const;

    NavCell cellAt(const de::Vector2D<float>& position) const;

    /// The top-left corner of a cell. A body the size of a tile sits exactly
    /// on this point, which is what makes waypoints tile-aligned.
    de::Vector2D<float> positionOf(NavCell cell) const;

    /// Four-connected breadth-first search from `from`.
    NavField flood(NavCell from) const;

private:
    int index(NavCell cell) const;

    int m_columns = 0;
    int m_rows = 0;
    float m_tileSize = 0.0f;
    std::vector<char> m_solid;
};

#endif // GAME_SIM_NAV_GRID_HPP
