#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include "SDL2/SDL.h"
#include "components/animation_component.hpp"
#include "components/bottom_layer_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/collision_layer_component.hpp"
#include "components/follow_component.hpp"
#include "components/overlay_layer_component.hpp"
#include "components/player_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "components/tile_layer_component.hpp"
#include "components/velocity_component.hpp"
#include "scene.hpp"
#include "core/quadtree.hpp"
#include "systems/debug_system.hpp"
#include <iostream>

class InGameScene : public Scene {
private:
    std::vector<entt::entity> m_entities;
    void initializeQuadtrees(entt::registry& registry, int mapWidth, int mapHeight) {
        AABB tileBoundary{0, 0, mapWidth, mapHeight};
        AABB spriteBoundary{0, 0, mapWidth, mapHeight};

        // Crear instancias de Quadtrees para tiles y sprites
        auto bottomQuadtree = std::make_shared<BottomLayerQuadtree>(tileBoundary, 4);
        auto overlayQuadtree = std::make_shared<OverlayLayerQuadtree>(tileBoundary, 4);
        auto collisionQuadtree = std::make_shared<CollisionLayerQuadtree>(tileBoundary, 4);
        auto spriteQuadtree = std::make_shared<ObjectQuadtree>(spriteBoundary, 4);

        // Guardar las instancias en el contexto del registry
        registry.ctx().emplace<std::shared_ptr<BottomLayerQuadtree>>(bottomQuadtree);
        registry.ctx().emplace<std::shared_ptr<OverlayLayerQuadtree>>(overlayQuadtree);
        registry.ctx().emplace<std::shared_ptr<CollisionLayerQuadtree>>(collisionQuadtree);
        registry.ctx().emplace<std::shared_ptr<ObjectQuadtree>>(spriteQuadtree);
    }
    void populateTileQuadtree(entt::registry& registry) {
        // auto& tileQuadtree = registry.ctx().get<std::shared_ptr<TileQuadtree>>();
        // auto view = registry.view<TileComponent, TransformComponent>();
        // for (auto entity : view) {
        //     auto& transform = view.get<TransformComponent>(entity);
        //     tileQuadtree->insert(entity, transform);
        // }
        auto& bottomQuadtree = registry.ctx().get<std::shared_ptr<BottomLayerQuadtree>>();
        auto& overlayQuadtree = registry.ctx().get<std::shared_ptr<OverlayLayerQuadtree>>();
        auto& collisionQuadtree = registry.ctx().get<std::shared_ptr<CollisionLayerQuadtree>>();
        auto bView = registry.view<TileLayerComponent, BottomLayerComponent>();
        for (auto entity : bView) {
            auto& layer = bView.get<TileLayerComponent>(entity);
            for (auto tile : layer.tileEntities) {
                auto& transform = registry.get<TransformComponent>(tile);
                bottomQuadtree->insert(tile, transform);
            }
        }
        auto oView = registry.view<TileLayerComponent, OverlayLayerComponent>();
        for (auto entity : oView) {
            auto& layer = oView.get<TileLayerComponent>(entity);
            for (auto tile : layer.tileEntities) {
                auto& transform = registry.get<TransformComponent>(tile);
                overlayQuadtree->insert(tile, transform);
            }
        }
        auto cView = registry.view<TileLayerComponent, CollisionLayerComponent>();
        for (auto entity : cView) {
            auto& layer = cView.get<TileLayerComponent>(entity);
            for (auto tile : layer.tileEntities) {
                auto& transform = registry.get<TransformComponent>(tile);
                collisionQuadtree->insert(tile, transform);
            }
        }
    }
    void populateSpriteQuadtree(entt::registry& registry) {
        auto& spriteQuadtree = registry.ctx().get<std::shared_ptr<ObjectQuadtree>>();

        auto view = registry.view<TransformComponent, SpriteComponent>();
        for (auto entity : view) {
            auto& transform = view.get<TransformComponent>(entity);
            spriteQuadtree->insert(entity, transform);
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
        auto& playerTransform = registry.get<TransformComponent>(player).position;
        registry.emplace<TransformComponent>(camera, playerTransform);
        registry.emplace<FollowComponent>(camera, player);
        m_entities.push_back(camera);
    }

    void initializeDebug(entt::registry& registry, bool open) {
        if(!open) return;
        auto &debugSystem = *registry.ctx().get<std::shared_ptr<DebugSystem>>();
        debugSystem.setOpen(open);
        debugSystem.register_component<PlayerComponent>("Player");
        debugSystem.register_component<CameraComponent>("Camera");
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