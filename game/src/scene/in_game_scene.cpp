#include "scene/in_game_scene.hpp"
#include "loaders/entity_loader.hpp" 
#include "world/level_parser.hpp"
#include "components/level_component.hpp"
#include <iostream>
// Este método se ejecuta cuando se entra a la escena
void InGameScene::onEnter(entt::registry& registry) {
    // Crear las entidades necesarias para el juego
    // Por ejemplo, cargar un jugador desde un archivo XML
    // Puedes inicializar otros sistemas o componentes específicos del nivel aquí
    SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
    // Cargar el jugador
    EntityLoader::loadPlayerDataFromXML(registry, renderer);
    // Cargar el nivel
    LevelParser levelParser;
    Level* level = levelParser.parseLevel(registry, "../assets/maps/dungeon1.tmx");
    auto levelEntity = registry.create();
    registry.emplace<LevelComponent>(levelEntity, level);
    m_entities.push_back(levelEntity);
    std::cout << "Level loaded" << std::endl;
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