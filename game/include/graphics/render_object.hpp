#ifndef RENDER_OBJECT_HPP
#define RENDER_OBJECT_HPP

#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"

class RenderObject : public Render {
public:
    void draw(entt::registry& registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) override;
};
#endif // RENDER_OBJECT_HPP