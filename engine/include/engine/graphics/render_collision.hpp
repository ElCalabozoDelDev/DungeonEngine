#ifndef DE_GRAPHICS_RENDER_COLLISION_HPP
#define DE_GRAPHICS_RENDER_COLLISION_HPP
#include "entt/entity/fwd.hpp"
#include <engine/graphics/render.hpp>

namespace de
{
class RenderCollision : public Render
{
public:
    void draw(entt::registry& registry, Box<float> cameraView, float offsetX,
              float offsetY, float zoomLevel) override;
};

} // namespace de

#endif // DE_GRAPHICS_RENDER_COLLISION_HPP
