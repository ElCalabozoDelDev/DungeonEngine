#include "scene/in_game_scene.hpp"
#include "loaders/entity_loader.hpp" 

// Este método se ejecuta cuando se entra a la escena
void InGameScene::onEnter(entt::registry& registry) {
    // Crear las entidades necesarias para el juego
    // Por ejemplo, cargar un jugador desde un archivo XML
    SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
    EntityLoader::loadPlayerDataFromXML("../assets/game.xml", registry, renderer);

    // Puedes inicializar otros sistemas o componentes específicos del nivel aquí
}

// Este método se llama en cada frame para actualizar la escena
void InGameScene::onUpdate(entt::registry& registry) {
    // Aquí iría la lógica del juego, como manejar colisiones, actualizar la física, etc.

    // Ejemplo: Verificar el estado del jugador

    // Se puede agregar lógica adicional para manejar enemigos, eventos de juego, etc.
}

// Este método se ejecuta cuando se sale de la escena
void InGameScene::onExit(entt::registry& registry) {
    // Destruir todas las entidades creadas en esta escena
    for (auto entity : m_entities) {
        registry.destroy(entity);
    }
    m_entities.clear();
}