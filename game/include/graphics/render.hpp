#ifndef RENDER_HPP
#define RENDER_HPP

#include "core/quadtree.hpp"
#include "entt/entity/fwd.hpp"

class Render {
public:
    virtual void render(entt::registry registry, AABB cameraView) = 0;
};

#endif // RENDER_HPP