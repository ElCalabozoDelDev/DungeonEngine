#include <engine/core/paused.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/widgets/widget.hpp>
#include <game/scene/menu_scene.hpp>
#include <game/widgets/menu_widget.hpp>
#include <memory>

using namespace de;

void MenuScene::onEnter(entt::registry& registry)
{
    // Nothing is simulated here, and leaving the level's trees behind would
    // keep handles to entities the previous scene destroyed.
    registry.ctx().get<SpatialIndex>().clear();
    registry.ctx().get<Paused>().value = false;

    auto entity = registry.create();
    registry.emplace<Widget>(entity, std::make_unique<MenuWidget>());
    m_entities.push_back(entity);
}

void MenuScene::onUpdate(entt::registry& registry) {}

void MenuScene::onExit(entt::registry& registry)
{
    for (auto entity : m_entities)
    {
        if (registry.valid(entity))
        {
            registry.destroy(entity);
        }
    }
    m_entities.clear();
}
