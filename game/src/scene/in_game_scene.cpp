#include "scene/in_game_scene.hpp"
#include "loaders/config.hpp"
#include "world/level_parser.hpp"
#include "components/level_component.hpp"
// Este método se ejecuta cuando se entra a la escena
void InGameScene::onEnter(entt::registry& registry) {
    // Crear las entidades necesarias para el juego
    // Por ejemplo, cargar un jugador desde un archivo XML
    // Puedes inicializar otros sistemas o componentes específicos del nivel aquí
    // Todo: Refactorizar para que se cargue el jugador y el nivel desde un archivo XML
    SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
    auto config = registry.ctx().get<Config>();
    // Cargar el jugador
    // EntityLoader::loadPlayerDataFromXML(registry, renderer);
    // Cargar el nivel
    // Todo: desacoplar logica de level y dejarlo mas cercano a ECS
    LevelParser levelParser;
    Level* level = levelParser.parseLevel(registry, config.levels["level1"].c_str());
    auto levelEntity = registry.create();
    auto & levelComponent = registry.emplace<LevelComponent>(levelEntity);
    levelComponent.tilesets = *level->getTilesets();
    levelComponent.layers = *level->getLayers();
    m_entities.push_back(levelEntity);
    for (auto tilesetEntity : levelComponent.tilesets) {
        m_entities.push_back(tilesetEntity);
    }
    for (auto layerEntity : levelComponent.layers) {
        m_entities.push_back(layerEntity);
    }
}

// Este método se llama en cada frame para actualizar la escena
void InGameScene::onUpdate(entt::registry& registry) {
    // Aquí iría la lógica del juego, como manejar colisiones, actualizar la física, etc.

    // Ejemplo: Verificar el estado del jugador

    // Se puede agregar lógica adicional para manejar enemigos, eventos de juego, etc.
    /*     
    auto view = registry.view<LevelComponent>();
    
    for (auto entity : view) {
        auto& levelComponent = view.get<LevelComponent>(entity);
        for (auto layer : levelComponent.layers) {
            auto tileLayer = registry.get<TileLayerComponent>(layer);
            tileLayer.tileLayer->update();
        }
    }
    */
}

// Este método se ejecuta cuando se sale de la escena
void InGameScene::onExit(entt::registry& registry) {
    // Destruir todas las entidades creadas en esta escena
    for (auto entity : m_entities) {
        registry.destroy(entity);
    }
    m_entities.clear();
}