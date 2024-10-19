#ifndef RENDER_HPP
#define RENDER_HPP

#include "entt/entity/fwd.hpp"
#include "Quadtree.hpp"
class Render {
public:
    virtual void draw(entt::registry &registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) = 0;
    virtual ~Render() = default;
};

#endif // RENDER_HPP