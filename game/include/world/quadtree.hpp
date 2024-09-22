#ifndef QUADTREE_HPP
#define QUADTREE_HPP

#include "components/sprite_component.hpp"
#include "components/tile_layer_component.hpp"
#include "entt/entt.hpp"
#include "components/position_component.hpp"

struct AABB {
    int x, y, width, height;

    bool contains(int px, int py) const {
        return px >= x && px <= (x + width) && py >= y && py <= (y + height);
    }

    bool intersects(const AABB& range) const {
        
        return !(range.x > (x + width) || 
                 (range.x + range.width) < x || 
                 range.y > (y + height) ||
                 (range.y + range.height) < y);
    }
};

template<typename T>
class Quadtree {
private:
    AABB boundary;
    int capacity;
    std::vector<entt::entity> tiles;
    std::unique_ptr<Quadtree> northeast, northwest, southeast, southwest;
    bool divided = false;

public:
    int getBoundaryX() const { return boundary.x; }
    int getBoundaryY() const { return boundary.y; }
    int getBoundaryWidth() const { return boundary.width; }
    int getBoundaryHeight() const { return boundary.height; }
    Quadtree(const AABB& boundary, int capacity)
        : boundary(boundary), capacity(capacity) {}

    bool insert(entt::entity tile, const PositionComponent& pos) {
        if (!boundary.contains(pos.position.m_x, pos.position.m_y)) {
            return false; // Fuera de los límites
        }

        if (tiles.size() < capacity) {
            tiles.push_back(tile);
            return true;
        }

        if (!divided) {
            subdivide();
        }

        // Intentar insertar en los nodos hijos
        if (northeast->insert(tile, pos)) return true;
        if (northwest->insert(tile, pos)) return true;
        if (southeast->insert(tile, pos)) return true;
        if (southwest->insert(tile, pos)) return true;

        return false;
    }

    bool remove(entt::entity entity, const PositionComponent& pos) {
        if (!boundary.contains(pos.position.m_x, pos.position.m_y)) {
            return false;  // Fuera de los límites, no puede estar aquí
        }

        // Intentar eliminar la entidad del nodo actual
        auto it = std::find(tiles.begin(), tiles.end(), entity);
        if (it != tiles.end()) {
            tiles.erase(it);
            return true;
        }
        // Si el nodo está dividido, intentar eliminar de los nodos hijos
        if (divided) {
            if (northeast->remove(entity, pos)) return true;
            if (northwest->remove(entity, pos)) return true;
            if (southeast->remove(entity, pos)) return true;
            if (southwest->remove(entity, pos)) return true;
        }

        return false;  // No se encontró la entidad en este Quadtree
    }

    void query(const AABB& range, std::vector<entt::entity>& found, const entt::registry& registry) const {
        if (!boundary.intersects(range)) {
            return; // No hay intersección con el rango de búsqueda
        }

        // Buscar en las entidades del nodo actual
        for (auto& tile : tiles) {
            auto& pos = registry.get<PositionComponent>(tile);
            if (range.contains(pos.position.m_x, pos.position.m_y)) {
                found.push_back(tile);
            }
        }

        // Si está subdividido, buscar en los nodos hijos
        if (divided) {
            northeast->query(range, found, registry);
            northwest->query(range, found, registry);
            southeast->query(range, found, registry);
            southwest->query(range, found, registry);
        }
    }

private:
    void subdivide() {
        int x = boundary.x;
        int y = boundary.y;
        int halfWidth = boundary.width / 2;
        int halfHeight = boundary.height / 2;

        northeast = std::make_unique<Quadtree>(AABB{x + halfWidth, y, halfWidth, halfHeight}, capacity);
        northwest = std::make_unique<Quadtree>(AABB{x, y, halfWidth, halfHeight}, capacity);
        southeast = std::make_unique<Quadtree>(AABB{x + halfWidth, y + halfHeight, halfWidth, halfHeight}, capacity);
        southwest = std::make_unique<Quadtree>(AABB{x, y + halfHeight, halfWidth, halfHeight}, capacity);

        divided = true;
    }
};

using TileQuadtree = Quadtree<TileLayerComponent>;
using SpriteQuadtree = Quadtree<SpriteComponent>;

#endif // QUADTREE_HPP