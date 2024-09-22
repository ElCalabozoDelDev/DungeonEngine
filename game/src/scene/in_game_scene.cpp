#include "scene/in_game_scene.hpp"
#include "loaders/config.hpp"
#include "world/level_parser.hpp"
#include "components/level_component.hpp"

// Este método se ejecuta cuando se entra a la escena
void InGameScene::onEnter(entt::registry& registry) {
    // Inicialización del juego
    SDL_Renderer* renderer = registry.ctx().get<SDL_Renderer*>();
    auto config = registry.ctx().get<Config>();

    // Siguiendo al jugador
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

    // Inicializar la cámara
    initializeCamera(registry, levelParser.getWidth() * levelParser.getTileSize(), levelParser.getHeight() * levelParser.getTileSize());
    
    initializeQuadtrees(registry, config.screenWidth, config.screenHeight);
    populateTileQuadtree(registry);
    populateSpriteQuadtree(registry);
}

// Este método se llama en cada frame para actualizar la escena
void InGameScene::onUpdate(entt::registry& registry) {
}

// Este método se ejecuta cuando se sale de la escena
void InGameScene::onExit(entt::registry& registry) {
    // Destruir todas las entidades creadas en esta escena
    for (auto entity : m_entities) {
        registry.destroy(entity);
    }
    m_entities.clear();
}