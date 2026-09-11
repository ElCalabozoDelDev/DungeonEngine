#ifndef DE_GRAPHICS_RENDER_BOTTOM_HPP
#define DE_GRAPHICS_RENDER_BOTTOM_HPP
#include "entt/entity/fwd.hpp"
#include <engine/graphics/render.hpp>
#include <engine/spatial/quadtree.hpp>

namespace de
{
class RenderBottom : public Render
{
public:
    void draw(entt::registry& registry, Box<float> cameraView, float offsetX,
              float offsetY, float zoomLevel) override;
};

} // namespace de

#endif // DE_GRAPHICS_RENDER_BOTTOM_HPP
