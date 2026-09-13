#ifndef GAME_SCENE_TITLE_SCENE_HPP
#define GAME_SCENE_TITLE_SCENE_HPP

#include <engine/scene/scene.hpp>
#include <entt/entity/fwd.hpp>
#include <vector>

class TitleScene : public de::Scene
{
public:
    void onEnter(entt::registry& registry) override;
    void onUpdate(entt::registry& registry) override;
    void onExit(entt::registry& registry) override;

private:
    std::vector<entt::entity> m_entities;
};

#endif // GAME_SCENE_TITLE_SCENE_HPP
