#ifndef GAME_SCENE_TITLE_SCENE_HPP
#define GAME_SCENE_TITLE_SCENE_HPP

#include <engine/scene/scene.hpp>

class TitleScene : public de::Scene
{
public:
    void onEnter(entt::registry& registry) override;
};

#endif // GAME_SCENE_TITLE_SCENE_HPP
