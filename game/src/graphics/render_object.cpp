#include "graphics/render_object.hpp"
#include "graphics/renderer.hpp"
#include "core/constants.hpp"

void RenderObject::draw(entt::registry &registry, AABB cameraView) {
    // Obtener Quadtrees desde el contexto
    auto &objectQuadtree = registry.ctx().get<std::vector<std::shared_ptr<Quadtree>>>()[LayerType::OBJECT];

    std::vector<entt::entity> visibleSprites;
    objectQuadtree->query(cameraView, visibleSprites, registry);
    Renderer::renderSprites(registry, visibleSprites);
}