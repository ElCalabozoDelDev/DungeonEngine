#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include "SDL2/SDL.h"
#include "components/animation_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/follow_component.hpp"
#include "components/player_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/velocity_component.hpp"
#include "scene.hpp"
#include "core/quadtree.hpp"
#include <iostream>

class InGameScene : public Scene {
private:
    std::vector<entt::entity> m_entities;
    void initializeQuadtrees(entt::registry& registry, int mapWidth, int mapHeight) {
        AABB tileBoundary{0, 0, mapWidth, mapHeight};
        AABB spriteBoundary{0, 0, mapWidth, mapHeight};

        // Crear instancias de Quadtrees para tiles y sprites
        auto tileQuadtree = std::make_shared<TileQuadtree>(tileBoundary, 4);
        auto spriteQuadtree = std::make_shared<SpriteQuadtree>(spriteBoundary, 4);

        // Guardar las instancias en el contexto del registry
        registry.ctx().emplace<std::shared_ptr<TileQuadtree>>(tileQuadtree);
        registry.ctx().emplace<std::shared_ptr<SpriteQuadtree>>(spriteQuadtree);
    }
    void populateTileQuadtree(entt::registry& registry) {
        auto& tileQuadtree = registry.ctx().get<std::shared_ptr<TileQuadtree>>();
        auto view = registry.view<TileComponent, PositionComponent>();
        for (auto entity : view) {
            auto& pos = view.get<PositionComponent>(entity);
            tileQuadtree->insert(entity, pos);
        }
    }
    void populateSpriteQuadtree(entt::registry& registry) {
        auto& spriteQuadtree = registry.ctx().get<std::shared_ptr<SpriteQuadtree>>();

        auto view = registry.view<PositionComponent, SpriteComponent>();
        for (auto entity : view) {
            auto& pos = view.get<PositionComponent>(entity);
            spriteQuadtree->insert(entity, pos);
        }
    }

    void initializeCamera(entt::registry& registry, int mapWidth, int mapHeight) {

        auto camera = registry.create();
        registry.emplace<CameraComponent>(camera);
        registry.emplace<CameraBoundsComponent>(camera, mapWidth, mapHeight);
        auto view = registry.view<PlayerComponent>();
        if(view.empty()) {
            std::cerr << "No player found in the scene" << std::endl;
            return;
        }
        auto player = *view.begin();
        auto& playerPosition = registry.get<PositionComponent>(player).position;
        registry.emplace<PositionComponent>(camera, playerPosition);
        registry.emplace<FollowComponent>(camera, player);
        m_entities.push_back(camera);
    }


    public:
        // Constructor por defecto
        InGameScene() = default;

        // Destructor
        virtual ~InGameScene() override = default;

        void onEnter(entt::registry& registry) override;

        void onUpdate(entt::registry& registry) override;

        void onExit(entt::registry& registry) override;
    };

#endif // IN_GAME_SCENE_HPP