#ifndef RENDER_HPP
#define RENDER_HPP

#include "core/quadtree.hpp"
#include "entt/entity/fwd.hpp"

class Render {
public:
    virtual void draw(entt::registry &registry, AABB cameraView, float offsetX, float offsetY, float zoomLevel) = 0;
    virtual ~Render() = default;
};

#endif // RENDER_HPP