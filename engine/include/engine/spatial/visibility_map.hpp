#ifndef DE_SPATIAL_VISIBILITY_MAP_HPP
#define DE_SPATIAL_VISIBILITY_MAP_HPP

#include <cstdint>
#include <vector>

namespace de
{
/// Optional per-tile visibility held in the registry context.
///
/// When present and `enabled`, render passes treat tiles/sprites according to
/// explored / currently-visible bits. Absence means "draw everything" so TMX
/// tooling and tests that never install a map keep working.
struct VisibilityMap
{
    enum class Appearance : std::uint8_t
    {
        Hidden,
        Visible,
        Remembered,
    };

    int columns = 0;
    int rows = 0;
    float tileSize = 16.0f;
    bool enabled = false;
    /// 1 = blocks line of sight (walls / void).
    std::vector<std::uint8_t> opaque;
    std::vector<std::uint8_t> visible;
    std::vector<std::uint8_t> explored;

    void reset(int cols, int rowCount, float size)
    {
        columns = cols;
        rows = rowCount;
        tileSize = size;
        const std::size_t n =
            static_cast<std::size_t>(cols) * static_cast<std::size_t>(rowCount);
        opaque.assign(n, 1);
        visible.assign(n, 0);
        explored.assign(n, 0);
        enabled = cols > 0 && rowCount > 0;
    }

    bool inBounds(int column, int row) const
    {
        return column >= 0 && row >= 0 && column < columns && row < rows;
    }

    int index(int column, int row) const { return row * columns + column; }

    void setOpaque(int column, int row, bool blocksSight)
    {
        if (!inBounds(column, row))
        {
            return;
        }
        opaque[static_cast<std::size_t>(index(column, row))] =
            blocksSight ? 1 : 0;
    }

    Appearance appearanceAt(float worldX, float worldY) const
    {
        if (!enabled || columns <= 0 || rows <= 0 || tileSize <= 0.0f)
        {
            return Appearance::Visible;
        }
        const int column = static_cast<int>(worldX / tileSize);
        const int row = static_cast<int>(worldY / tileSize);
        if (!inBounds(column, row))
        {
            return Appearance::Hidden;
        }
        const int i = index(column, row);
        if (visible[static_cast<std::size_t>(i)] != 0)
        {
            return Appearance::Visible;
        }
        if (explored[static_cast<std::size_t>(i)] != 0)
        {
            return Appearance::Remembered;
        }
        return Appearance::Hidden;
    }
};

} // namespace de

#endif // DE_SPATIAL_VISIBILITY_MAP_HPP
