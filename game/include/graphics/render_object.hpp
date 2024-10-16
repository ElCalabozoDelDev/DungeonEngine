#ifndef RENDER_OBJECT_HPP
#define RENDER_OBJECT_HPP

#include "core/quadtree.hpp"
#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"

class RenderObject : public Render {
public:
    void draw(entt::registry& registry, AABB cameraView, float offsetX, float offsetY, float zoomLevel) override;
};
#endif // RENDER_OBJECT_HPP