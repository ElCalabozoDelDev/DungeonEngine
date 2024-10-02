#ifndef QUADTREE_MANAGER_HPP
#define QUADTREE_MANAGER_HPP

#include "core/quadtree.hpp"
#include <vector>
#include <memory>
#include <entt/entt.hpp>

class QuadtreeManager {
public:
    enum LayerType {
        BOTTOM,
        OVERLAY,
        COLLISION,
        OBJECT
    };

    // Constructor
    QuadtreeManager(int mapWidth, int mapHeight) {
        AABB boundary{0, 0, mapWidth, mapHeight};
        quadtrees.resize(4);
        quadtrees[BOTTOM] = std::make_shared<Quadtree<BottomLayerComponent>>(boundary, 4);
        quadtrees[OVERLAY] = std::make_shared<Quadtree<OverlayLayerComponent>>(boundary, 4);
        quadtrees[COLLISION] = std::make_shared<Quadtree<CollisionLayerComponent>>(boundary, 4);
        quadtrees[OBJECT] = std::make_shared<Quadtree<SpriteComponent>>(boundary, 4);
    }

    // Insertar tiles en el quadtree adecuado
    template<typename T>
    void insertTile(LayerType type, entt::entity entity, const TransformComponent& transform) {
        std::static_pointer_cast<Quadtree<T>>(quadtrees[type])->insert(entity, transform);
    }

    // Insertar sprites en el quadtree de objetos
    void insertSprite(entt::entity entity, const TransformComponent& transform) {
        std::static_pointer_cast<Quadtree<SpriteComponent>>(quadtrees[OBJECT])->insert(entity, transform);
    }

    // Consulta de tiles visibles
    template<typename T>
    void queryVisible(LayerType type, const AABB& view, std::vector<entt::entity>& entities, entt::registry& registry) {
        std::static_pointer_cast<Quadtree<T>>(quadtrees[type])->query(view, entities, registry);
    }

    // Consulta de sprites visibles
    void queryVisibleSprites(const AABB& view, std::vector<entt::entity>& entities, entt::registry& registry) {
        std::static_pointer_cast<Quadtree<SpriteComponent>>(quadtrees[OBJECT])->query(view, entities, registry);
    }

    // Método para obtener el quadtree de una capa
    template<typename T>
    std::shared_ptr<Quadtree<T>> getQuadtree(LayerType type) {
        return std::static_pointer_cast<Quadtree<T>>(quadtrees[type]);
    }

private:
    std::vector<std::shared_ptr<void>> quadtrees;  // Array de quadtrees genéricos
};

#endif // QUADTREE_MANAGER_HPP