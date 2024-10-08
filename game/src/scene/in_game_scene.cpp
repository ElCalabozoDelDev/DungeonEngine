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
    LevelParser levelParser = LevelParser(&m_entities);
    levelParser.parseLevel(registry, config.levels["level1"].c_str());


    // Inicializar la cámara
    int mapWidth = levelParser.getWidth() * levelParser.getTileSize();
    int mapHeight = levelParser.getHeight() * levelParser.getTileSize();
    initializeCamera(registry, mapWidth, mapHeight);
    
    initializeQuadtrees(registry, mapWidth, mapHeight);
    populateTileQuadtree(registry);
    populateSpriteQuadtree(registry);
    initializeRenderers(registry);
    initializeDebug(registry, config.debug);
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
    for (auto quadtree : m_quadtrees) {
        quadtree->clear();
    }
    m_quadtrees.clear();
    m_entities.clear();
}