#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP


#include "loaders/config.hpp"
#include "scene.hpp"
#include <components/dimension_component.hpp>
#include <entt/entity/fwd.hpp>
#include <vector>

class InGameScene : public Scene {
public:
    InGameScene();
    virtual ~InGameScene() override = default;

    void onEnter(entt::registry& registry) override;
    void onUpdate(entt::registry& registry) override;
    void onExit(entt::registry& registry) override;

private:
    std::vector<entt::entity> m_entities;

    void initializeQuadtrees(entt::registry& registry, float mapWidth, float mapHeight);
    void populateTileQuadtree(entt::registry& registry);
    void populateSpriteQuadtree(entt::registry& registry);
    void initializeCamera(entt::registry& registry, int mapWidth, int mapHeight, Config config);
    void initializeRenderers(entt::registry& registry);
    void initializeDebug(entt::registry& registry, bool open);
};

#endif // IN_GAME_SCENE_HPP
