#include <engine/spatial/spatial_index.hpp>
#include <engine/widgets/widget.hpp>
#include <game/play_state.hpp>
#include <game/scene/options_scene.hpp>
#include <game/widgets/options_widget.hpp>
#include <memory>

using namespace de;

void OptionsScene::onEnter(entt::registry& registry)
{
    registry.ctx().get<SpatialIndex>().clear();
    resetRunPresentation(registry);

    auto entity = registry.create();
    registry.emplace<Widget>(entity, std::make_unique<OptionsWidget>());
    m_entities.push_back(entity);
}

void OptionsScene::onUpdate(entt::registry& /*registry*/) {}

void OptionsScene::onExit(entt::registry& registry)
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
