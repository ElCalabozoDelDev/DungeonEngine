#include "graphics/render_object.hpp"
#include "graphics/renderer.hpp"

void RenderObject::draw(entt::registry &registry, AABB cameraView) {
    // Obtener Quadtrees desde el contexto
    auto &objectQuadtree = registry.ctx().get<std::shared_ptr<ObjectQuadtree>>();

    std::vector<entt::entity> visibleSprites;
    objectQuadtree->query(cameraView, visibleSprites, registry);
    Renderer::renderSprites(registry, visibleSprites);
}