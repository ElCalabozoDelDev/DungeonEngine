#ifndef GAME_SCENE_IN_GAME_SCENE_HPP
#define GAME_SCENE_IN_GAME_SCENE_HPP

#include <engine/loaders/config.hpp>
#include <engine/scene/scene.hpp>
#include <entt/entity/fwd.hpp>
#include <string>
#include <vector>

class InGameScene : public de::Scene
{
public:
    InGameScene();
    ~InGameScene() override = default;

    void onEnter(entt::registry& registry) override;
    void onUpdate(entt::registry& registry) override;
    void onExit(entt::registry& registry) override;

private:
    std::vector<entt::entity> m_entities;
    bool m_debugOpen = false;

    static void fail(entt::registry& registry, const std::string& reason);
    bool loadFloor(entt::registry& registry, int floorIndex,
                   const de::Config& config, bool preserveRun);
    void clearLevelEntities(entt::registry& registry);
    void tagObjectsByType(entt::registry& registry);
    void initializeVisibility(entt::registry& registry, int columns, int rows,
                              float tileSize);
    void initializeQuadtrees(entt::registry& registry, float mapWidth,
                             float mapHeight);
    void populateTileQuadtree(entt::registry& registry);
    void populateSpriteQuadtree(entt::registry& registry);
    void initializeCamera(entt::registry& registry, int mapWidth, int mapHeight,
                          const de::Config& config);
    void initializeRenderers(entt::registry& registry);
    void initializeHud(entt::registry& registry);
    void initializeDebug(entt::registry& registry, bool open);
};

#endif // GAME_SCENE_IN_GAME_SCENE_HPP
