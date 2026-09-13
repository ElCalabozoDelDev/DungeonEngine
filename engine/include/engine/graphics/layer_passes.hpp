#ifndef DE_GRAPHICS_LAYER_PASSES_HPP
#define DE_GRAPHICS_LAYER_PASSES_HPP

#include <engine/graphics/render.hpp>
#include <engine/spatial/spatial_index.hpp>

namespace de
{
/// Draws the tiles filed in one layer of the SpatialIndex that the camera can
/// see.
///
/// `cullMargin` grows the visible rectangle by that many world units, so a
/// tile straddling the edge of the view is still drawn: the size of one tile
/// is enough.
class TileLayerPass final : public Render
{
public:
    TileLayerPass(Layer layer, float cullMargin)
        : m_layer(layer), m_cullMargin(cullMargin)
    {
    }

    void draw(entt::registry& registry, const Camera2D& camera) override;

private:
    Layer m_layer;
    float m_cullMargin;
};

/// Draws the sprites filed in one layer of the SpatialIndex that the camera
/// can see. `cullMargin` as for TileLayerPass: the size of the largest sprite.
class SpriteLayerPass final : public Render
{
public:
    SpriteLayerPass(Layer layer, float cullMargin)
        : m_layer(layer), m_cullMargin(cullMargin)
    {
    }

    void draw(entt::registry& registry, const Camera2D& camera) override;

private:
    Layer m_layer;
    float m_cullMargin;
};

} // namespace de

#endif // DE_GRAPHICS_LAYER_PASSES_HPP
