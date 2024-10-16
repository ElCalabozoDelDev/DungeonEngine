#ifndef RENDER_COLLISION_HPP
#define RENDER_COLLISION_HPP

#include "core/quadtree.hpp"
#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"

class RenderCollision : public Render {
public:
    void draw(entt::registry& registry, AABB cameraView, float offsetX, float offsetY) override;
};
#endif // RENDER_COLLISION_HPP