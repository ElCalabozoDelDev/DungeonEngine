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

    static void fail(entt::registry& registry, const std::string& reason);

    /// Loads the arena and records its size in GameState. False, with a
    /// StartupError reported, when it cannot.
    bool loadLevel(entt::registry& registry, const de::Config& config);
    static void startMusic(entt::registry& registry);
    /// The snake head: every Tiled "Player" object, or one at the room
    /// centre when the map has none.
    void spawnPlayer(entt::registry& registry);
    void spawnBat(entt::registry& registry);
    void initializeCamera(entt::registry& registry, const de::Config& config);
    void initializeRenderers(entt::registry& registry);
    void initializeHud(entt::registry& registry);
    void initializeDebug(entt::registry& registry, bool open);
};

#endif // GAME_SCENE_IN_GAME_SCENE_HPP
