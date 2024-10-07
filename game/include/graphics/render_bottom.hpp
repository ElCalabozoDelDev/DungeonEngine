#ifndef RENDER_BOTTOM_HPP
#define RENDER_BOTTOM_HPP

#include "core/quadtree.hpp"
#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"

class RenderBottom : public Render {
public:
    void draw(entt::registry &registry, AABB cameraView) override;
};

#endif // RENDER_BOTTOM_HPP