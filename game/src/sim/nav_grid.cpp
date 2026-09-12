#include <algorithm>
#include <cmath>
#include <deque>
#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/collision_layer_component.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/overlay_layer_component.hpp>
#include <engine/components/tile_layer_component.hpp>
#include <engine/components/transform_component.hpp>
#include <game/sim/nav_grid.hpp>

using namespace de;

namespace
{
/// Neighbours in a fixed order, so two runs produce the same path. Any
/// ordering works; what matters is that it never varies.
constexpr int NeighbourColumn[] = {1, -1, 0, 0};
constexpr int NeighbourRow[] = {0, 0, 1, -1};

} // namespace

bool NavField::reachable(NavCell cell) const
{
    const int i = index(cell);
    return i >= 0 && m_distance[static_cast<std::size_t>(i)] >= 0;
}

int NavField::distance(NavCell cell) const
{
    const int i = index(cell);
    return i >= 0 ? m_distance[static_cast<std::size_t>(i)] : -1;
}

NavCell NavField::stepTowards(NavCell goal) const
{
    if (!reachable(goal))
    {
        return m_origin;
    }

    // Walk back along `previous` until the cell before the origin is reached;
    // that cell is the step to take.
    NavCell cell = goal;
    while (!(cell == m_origin))
    {
        const int previous = m_previous[static_cast<std::size_t>(index(cell))];
        if (previous < 0)
        {
            return m_origin;
        }
        const NavCell before{previous % m_columns, previous / m_columns};
        if (before == m_origin)
        {
            return cell;
        }
        cell = before;
    }
    return m_origin;
}

int NavField::index(NavCell cell) const
{
    if (cell.column < 0 || cell.row < 0 || cell.column >= m_columns ||
        cell.row >= m_rows)
    {
        return -1;
    }
    return cell.row * m_columns + cell.column;
}

void NavGrid::build(entt::registry& registry)
{
    // Collect the tiles of both solid layers first. Overlay counts as well as
    // Collision; see SolidLayers in CollisionSystem.
    std::vector<entt::entity> tiles;
    const auto gather = [&](const TileLayerComponent& layer)
    {
        for (auto tile : layer.tileEntities)
        {
            if (registry.valid(tile) &&
                registry.all_of<TransformComponent, DimensionComponent>(tile))
            {
                tiles.push_back(tile);
            }
        }
    };
    for (auto entity :
         registry.view<TileLayerComponent, CollisionLayerComponent>())
    {
        gather(registry.get<TileLayerComponent>(entity));
    }
    for (auto entity :
         registry.view<TileLayerComponent, OverlayLayerComponent>())
    {
        gather(registry.get<TileLayerComponent>(entity));
    }
    if (tiles.empty())
    {
        reset(0, 0, 0.0f);
        return;
    }

    // The tile size has to come from an actual tile, not from the first
    // entity that happens to carry a DimensionComponent: the camera carries
    // one too (800x600), and picking that up collapses the grid to a single
    // cell and makes every item look unreachable.
    const float tileSize =
        registry.get<DimensionComponent>(tiles.front()).width;
    if (tileSize <= 0.0f)
    {
        reset(0, 0, 0.0f);
        return;
    }

    int columns = 0;
    int rows = 0;
    if (const auto bounds = registry.view<CameraBoundsComponent>();
        bounds.begin() != bounds.end())
    {
        const auto& size = bounds.get<CameraBoundsComponent>(*bounds.begin());
        columns = static_cast<int>(std::lround(size.levelWidth / tileSize));
        rows = static_cast<int>(std::lround(size.levelHeight / tileSize));
    }

    std::vector<NavCell> solid;
    solid.reserve(tiles.size());
    for (auto tile : tiles)
    {
        const auto& position = registry.get<TransformComponent>(tile).position;
        solid.push_back(
            NavCell{static_cast<int>(std::floor(position.getX() / tileSize)),
                    static_cast<int>(std::floor(position.getY() / tileSize))});
    }

    if (columns <= 0 || rows <= 0)
    {
        for (const NavCell& cell : solid)
        {
            columns = std::max(columns, cell.column + 1);
            rows = std::max(rows, cell.row + 1);
        }
    }

    reset(columns, rows, tileSize);
    for (const NavCell& cell : solid)
    {
        setSolid(cell.column, cell.row, true);
    }
}

NavGrid& NavGrid::reset(int columns, int rows, float tileSize)
{
    m_columns = std::max(0, columns);
    m_rows = std::max(0, rows);
    m_tileSize = tileSize;
    m_solid.assign(static_cast<std::size_t>(m_columns) *
                       static_cast<std::size_t>(m_rows),
                   char{0});
    return *this;
}

NavGrid& NavGrid::setSolid(int column, int row, bool solid)
{
    const int i = index(NavCell{column, row});
    if (i >= 0)
    {
        m_solid[static_cast<std::size_t>(i)] = solid ? char{1} : char{0};
    }
    return *this;
}

bool NavGrid::inBounds(NavCell cell) const { return index(cell) >= 0; }

bool NavGrid::isSolid(NavCell cell) const
{
    const int i = index(cell);
    return i < 0 || m_solid[static_cast<std::size_t>(i)] != 0;
}

NavCell NavGrid::cellAt(const de::Vector2D<float>& position) const
{
    if (m_tileSize <= 0.0f)
    {
        return NavCell{};
    }
    return NavCell{static_cast<int>(std::floor(position.getX() / m_tileSize)),
                   static_cast<int>(std::floor(position.getY() / m_tileSize))};
}

de::Vector2D<float> NavGrid::positionOf(NavCell cell) const
{
    return de::Vector2D<float>(static_cast<float>(cell.column) * m_tileSize,
                               static_cast<float>(cell.row) * m_tileSize);
}

NavField NavGrid::flood(NavCell from) const
{
    NavField field;
    field.m_columns = m_columns;
    field.m_rows = m_rows;
    field.m_origin = from;
    const std::size_t count =
        static_cast<std::size_t>(m_columns) * static_cast<std::size_t>(m_rows);
    field.m_distance.assign(count, -1);
    field.m_previous.assign(count, -1);

    const int start = index(from);
    if (start < 0 || isSolid(from))
    {
        return field;
    }

    field.m_distance[static_cast<std::size_t>(start)] = 0;

    std::deque<NavCell> queue{from};
    while (!queue.empty())
    {
        const NavCell cell = queue.front();
        queue.pop_front();
        const int here = index(cell);

        for (int direction = 0; direction < 4; ++direction)
        {
            const NavCell next{cell.column + NeighbourColumn[direction],
                               cell.row + NeighbourRow[direction]};
            const int there = index(next);
            if (there < 0 || isSolid(next) ||
                field.m_distance[static_cast<std::size_t>(there)] >= 0)
            {
                continue;
            }
            field.m_distance[static_cast<std::size_t>(there)] =
                field.m_distance[static_cast<std::size_t>(here)] + 1;
            field.m_previous[static_cast<std::size_t>(there)] = here;
            queue.push_back(next);
        }
    }
    return field;
}

int NavGrid::index(NavCell cell) const
{
    if (cell.column < 0 || cell.row < 0 || cell.column >= m_columns ||
        cell.row >= m_rows)
    {
        return -1;
    }
    return cell.row * m_columns + cell.column;
}
