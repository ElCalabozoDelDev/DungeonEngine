#ifndef RENDER_BOTTOM_HPP
#define RENDER_BOTTOM_HPP

#include "entt/entity/fwd.hpp"
#include "graphics/render.hpp"
#include "Quadtree.hpp"

class RenderBottom : public Render {
public:
    void draw(entt::registry& registry, quadtree::Box<float> cameraView, float offsetX, float offsetY, float zoomLevel) override;
};

#endif // RENDER_BOTTOM_HPP