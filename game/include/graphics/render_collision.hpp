#ifndef RENDER_COLLISION_HPP
#define RENDER_COLLISION_HPP

#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"

class RenderCollision : public Render {
public:
    void draw(entt::registry& registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) override;
};
#endif // RENDER_COLLISION_HPP