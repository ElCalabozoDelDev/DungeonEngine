#ifndef DE_SCENE_SCENE_SYSTEM_HPP
#define DE_SCENE_SCENE_SYSTEM_HPP
#include <engine/scene/scene.hpp>
#include <engine/systems/system.hpp>

namespace de
{
class SceneSystem : public System
{
private:
    std::unique_ptr<Scene> m_currentScene;

public:
    void run(entt::registry& registry) override;

    void changeScene(entt::registry& registry, std::unique_ptr<Scene> newScene);
};

} // namespace de

#endif // DE_SCENE_SCENE_SYSTEM_HPP
