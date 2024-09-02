#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include "scene.hpp"
#include "components/position_component.hpp"
#include "components/texture_component.hpp"
#include "components/animation_component.hpp"
#include "entity_loader.hpp" // Include the header file that declares EntityLoader
// Incluir otros componentes y sistemas necesarios

class InGameScene : public Scene {
private:
    std::vector<entt::entity> m_entities; // Para rastrear las entidades creadas en esta escena

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