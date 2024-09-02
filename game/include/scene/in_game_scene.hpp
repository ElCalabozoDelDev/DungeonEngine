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

    // Este método se ejecuta cuando se entra a la escena
    void onEnter(entt::registry& registry) override {
        // Crear las entidades necesarias para el juego
        // Por ejemplo, cargar un jugador desde un archivo XML
        SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
        EntityLoader::loadPlayerDataFromXML("../assets/game.xml", registry, renderer);

        // Puedes inicializar otros sistemas o componentes específicos del nivel aquí
    }

    // Este método se llama en cada frame para actualizar la escena
    void onUpdate(entt::registry& registry) override {
        // Aquí iría la lógica del juego, como manejar colisiones, actualizar la física, etc.

        // Ejemplo: Verificar el estado del jugador
        

        // Se puede agregar lógica adicional para manejar enemigos, eventos de juego, etc.
    }

    // Este método se ejecuta cuando se sale de la escena
    void onExit(entt::registry& registry) override {
        // Destruir todas las entidades creadas en esta escena
        for (auto entity : m_entities) {
            registry.destroy(entity);
        }
        m_entities.clear();
    }
};

#endif // IN_GAME_SCENE_HPP