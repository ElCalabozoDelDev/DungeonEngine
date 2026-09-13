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

    auto entity = track(registry.create());
    registry.emplace<Widget>(entity, std::make_unique<OptionsWidget>());
}
