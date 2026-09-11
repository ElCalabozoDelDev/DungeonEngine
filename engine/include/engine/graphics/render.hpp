#ifndef DE_GRAPHICS_RENDER_HPP
#define DE_GRAPHICS_RENDER_HPP
#include "entt/entity/fwd.hpp"
#include <engine/spatial/quadtree.hpp>

namespace de
{
class Render
{
public:
    virtual void draw(entt::registry& registry, Box<float> cameraView,
                      float offsetX, float offsetY, float zoomLevel) = 0;
    virtual ~Render() = default;
};

} // namespace de

#endif // DE_GRAPHICS_RENDER_HPP
