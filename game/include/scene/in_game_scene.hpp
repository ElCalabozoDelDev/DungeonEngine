#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include "components/bottom_layer_component.hpp"
#include "components/camera_bounds_component.hpp"
#include "components/camera_component.hpp"
#include "components/collision_layer_component.hpp"
#include "components/follow_component.hpp"
#include "components/overlay_layer_component.hpp"
#include "components/player_component.hpp"
#include "components/sprite_component.hpp"
#include "components/tile_layer_component.hpp"
#include "core/constants.hpp"
#include "graphics/render_bottom.hpp"
#include "graphics/render_collision.hpp"
#include "graphics/render_object.hpp"
#include "graphics/render_overlay.hpp"
#include "loaders/config.hpp"
#include "quadtree.hpp"
#include "scene.hpp"
#include "systems/debug_system.hpp"
#include <components/dimension_component.hpp>
#include <entt/entity/fwd.hpp>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

class InGameScene : public Scene
{
private:
    std::function<TransformComponent(Node*)> getBox = [](Node* node)
    {
        return node->box;
    };
    std::vector<entt::entity> m_entities;

    void initializeQuadtrees(entt::registry& registry, float mapWidth,
                             float mapHeight)
    {
        Box<float> tileBoundary{0, 0, mapWidth, mapHeight};
        Box<float> spriteBoundary{0, 0, mapWidth, mapHeight};
        Node node = {{0, 0, mapWidth, mapHeight}, entt::null};
        m_quadtrees[LayerType::BOTTOM] =
            Quadtree<Node*, decltype(getBox)>(tileBoundary, 4, getBox);
        m_quadtrees[LayerType::OVERLAY] =
            std::make_shared<Quadtree<Node*, decltype(getBox)>>(tileBoundary, 4,
                                                                getBox);
        m_quadtrees[LayerType::COLLISION] =
            std::make_shared<Quadtree<Node*, decltype(getBox)>>(tileBoundary, 4,
                                                                getBox);
        m_quadtrees[LayerType::OBJECT] =
            std::make_shared<Quadtree<Node*, decltype(getBox)>>(spriteBoundary,
                                                                4, getBox);
        registry.ctx()
            .emplace<std::vector<
                std::shared_ptr<Quadtree<Node*, decltype(getBox)>>>>(
                m_quadtrees);
    }
    void populateTileQuadtree(entt::registry& registry)
    {
        auto bView = registry.view<TileLayerComponent, BottomLayerComponent>();
        for (auto entity : bView)
        {
            auto& layer = bView.get<TileLayerComponent>(entity);
            for (auto tile : layer.tileEntities)
            {
                auto& transform = registry.get<TransformComponent>(tile);
                Node node = {transform, tile};
                m_quadtrees[LayerType::BOTTOM]->add(transform, getBox);
            }
        }
        auto oView = registry.view<TileLayerComponent, OverlayLayerComponent>();
        for (auto entity : oView)
        {
            auto& layer = oView.get<TileLayerComponent>(entity);
            for (auto tile : layer.tileEntities)
            {
                auto& transform = registry.get<TransformComponent>(tile);
                m_quadtrees[LayerType::OVERLAY]->insert(tile, transform);
            }
        }
        auto cView =
            registry.view<TileLayerComponent, CollisionLayerComponent>();
        for (auto entity : cView)
        {
            auto& layer = cView.get<TileLayerComponent>(entity);
            for (auto tile : layer.tileEntities)
            {
                auto& transform = registry.get<TransformComponent>(tile);
                m_quadtrees[LayerType::COLLISION]->insert(tile, transform);
            }
        }
    }
    void populateSpriteQuadtree(entt::registry& registry)
    {
        auto view = registry.view<TransformComponent, SpriteComponent>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            m_quadtrees[LayerType::OBJECT]->add(entity, transform);
        }
    }

    void initializeCamera(entt::registry& registry, int mapWidth, int mapHeight,
                          Config config)
    {

        auto cameraEntity = registry.create();
        registry.emplace<DimensionComponent>(cameraEntity, config.cameraWidth,
                                             config.cameraHeight);
        auto& camera = registry.emplace<CameraComponent>(cameraEntity);
        camera.viewportOffsetX = config.viewportOffsetX;
        camera.viewportOffsetY = config.viewportOffsetY;
        camera.zoomLevel = config.zoomLevel;
        registry.emplace<CameraBoundsComponent>(cameraEntity, mapWidth,
                                                mapHeight);
        auto view = registry.view<PlayerComponent>();
        if (view.empty())
        {
            std::cerr << "No player found in the scene" << std::endl;
            return;
        }
        auto player = *view.begin();
        auto& playerTransform =
            registry.get<TransformComponent>(player).position;
        registry.emplace<TransformComponent>(cameraEntity, playerTransform);
        registry.emplace<FollowComponent>(cameraEntity, player);
        m_entities.push_back(cameraEntity);
    }

    void initializeRenderers(entt::registry& registry)
    {
        // registry renders from top to bottom
        auto bottomEntity = registry.create();
        auto overlayEntity = registry.create();
        auto collisionEntity = registry.create();
        auto objectEntity = registry.create();
        registry.emplace<std::shared_ptr<Render>>(
            collisionEntity, std::make_shared<RenderCollision>());
        registry.emplace<std::shared_ptr<Render>>(
            objectEntity, std::make_shared<RenderObject>());
        registry.emplace<std::shared_ptr<Render>>(
            overlayEntity, std::make_shared<RenderOverlay>());
        registry.emplace<std::shared_ptr<Render>>(
            bottomEntity, std::make_shared<RenderBottom>());
    }

    void initializeDebug(entt::registry& registry, bool open)
    {
        if (!open)
            return;
        auto& debugSystem = *registry.ctx().get<std::shared_ptr<DebugSystem>>();
        debugSystem.setOpen(open);
        debugSystem.register_component<PlayerComponent>("Player");
        debugSystem.register_component<CameraComponent>("Camera");
    }

public:
    // Constructor por defecto
    InGameScene()
    {
        m_quadtrees.resize(LayerType::COUNT);
    }

    // Destructor
    virtual ~InGameScene() override = default;

    void onEnter(entt::registry& registry) override;

    void onUpdate(entt::registry& registry) override;

    void onExit(entt::registry& registry) override;
};

#endif // IN_GAME_SCENE_HPP