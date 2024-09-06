#ifndef SCENE_SYSTEM_HPP
#define SCENE_SYSTEM_HPP

#include "systems/system.hpp"
#include "scene/scene.hpp"

class SceneSystem : public System {
private:
    std::unique_ptr<Scene> m_currentScene;

public:
    void run(entt::registry& registry) override;

    void changeScene(entt::registry& registry, std::unique_ptr<Scene> newScene);
};

#endif