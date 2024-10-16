#ifndef RENDER_OVERLAY_HPP
#define RENDER_OVERLAY_HPP

#include "core/quadtree.hpp"
#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"

class RenderOverlay : public Render {
public:
    void draw(entt::registry& registry, AABB cameraView, float offsetX, float offsetY, float zoomLevel) override;
};
#endif // RENDER_OVERLAY_HPP