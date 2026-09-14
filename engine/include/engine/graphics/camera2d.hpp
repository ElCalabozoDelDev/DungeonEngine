#ifndef DE_GRAPHICS_CAMERA2D_HPP
#define DE_GRAPHICS_CAMERA2D_HPP

#include <SDL.h>
#include <engine/core/vector_2d.hpp>
#include <engine/spatial/quadtree.hpp>
#include <entt/entt.hpp>
#include <optional>

namespace de
{
/// The one world-to-screen mapping in the engine.
///
/// Drawing and culling both go through this, so they cannot disagree. They
/// used to: the draw formula placed the camera centre at
/// `screenSize/2 + viewportOffset` using the *window* size, while the cull
/// rectangle was built from the *camera* size. With the shipped settings the
/// screen showed about 67 world units further right and 33 further down than
/// the cull rectangle covered, so tiles there were discarded and the edges of
/// the window stayed black.
///
/// Coordinates are in the renderer's logical space (SDL_RenderSetLogicalSize),
/// which SDL then scales to whatever the window happens to be.
struct Camera2D
{
    /// World point shown at the centre of the view.
    Vector2D<float> position{};

    /// World units are multiplied by this to get logical pixels.
    float zoom = 1.0f;

    /// Logical render size, in pixels.
    float viewWidth = 800.0f;
    float viewHeight = 600.0f;

    /// Half the visible area, in world units.
    Vector2D<float> halfExtents() const
    {
        return Vector2D<float>(viewWidth / (2.0f * zoom),
                               viewHeight / (2.0f * zoom));
    }

    SDL_Point worldToScreen(const Vector2D<float>& world) const
    {
        return SDL_Point{
            static_cast<int>((world.x - position.x) * zoom + viewWidth / 2.0f),
            static_cast<int>((world.y - position.y) * zoom +
                             viewHeight / 2.0f)};
    }

    /// The world rectangle currently on screen, grown by `margin` world units
    /// so that entities straddling the edge are still drawn.
    Box<float> visibleWorld(float margin = 0.0f) const
    {
        const Vector2D<float> half = halfExtents();
        return Box<float>(
            position.x - half.x - margin, position.y - half.y - margin,
            half.x * 2.0f + margin * 2.0f, half.y * 2.0f + margin * 2.0f);
    }
};

/// Builds a Camera2D from the first camera entity in the registry, or nullopt
/// when there is none (a scene that has not loaded yet).
std::optional<Camera2D> activeCamera(entt::registry& registry);

} // namespace de

#endif // DE_GRAPHICS_CAMERA2D_HPP
