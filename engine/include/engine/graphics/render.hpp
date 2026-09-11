#ifndef DE_GRAPHICS_RENDER_HPP
#define DE_GRAPHICS_RENDER_HPP

#include <engine/graphics/camera2d.hpp>
#include <entt/entity/fwd.hpp>
#include <memory>

namespace de
{
/// One pass of the frame: a layer of tiles, the sprites, a debug overlay.
///
/// Passes take the camera rather than a cull box plus two offsets plus a zoom;
/// those four arguments were four chances to disagree with the draw call.
class Render
{
public:
    virtual void draw(entt::registry& registry, const Camera2D& camera) = 0;
    virtual ~Render() = default;
};

/// A render pass attached to an entity, drawn in ascending `order`.
///
/// A component of its own rather than a bare std::shared_ptr<Render>: the
/// order used to be whatever order EnTT happened to iterate the pool in, and
/// the background drawing behind the overlay was luck.
struct RenderPass
{
    std::unique_ptr<Render> pass;
    int order = 0;
};

/// Conventional ordering for the built-in passes.
namespace render_order
{
constexpr int Bottom = 0;
constexpr int Collision = 100;
constexpr int Object = 200;
constexpr int Overlay = 300;
} // namespace render_order

} // namespace de

#endif // DE_GRAPHICS_RENDER_HPP
