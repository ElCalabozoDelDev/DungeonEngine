#ifndef DE_GRAPHICS_RENDER_OBJECT_HPP
#define DE_GRAPHICS_RENDER_OBJECT_HPP

#include <engine/graphics/render.hpp>
#include <entt/entity/fwd.hpp>

namespace de
{
class RenderObject : public Render
{
public:
    void draw(entt::registry& registry, const Camera2D& camera) override;
};

} // namespace de

#endif // DE_GRAPHICS_RENDER_OBJECT_HPP
