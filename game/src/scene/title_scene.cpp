#include <engine/audio/audio_manager.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/widgets/widget.hpp>
#include <game/play_state.hpp>
#include <game/scene/title_scene.hpp>
#include <game/widgets/title_widget.hpp>
#include <memory>

using namespace de;

void TitleScene::onEnter(entt::registry& registry)
{
    registry.ctx().get<SpatialIndex>().clear();
    resetRunPresentation(registry);

    // Assets were loaded at startup (game::loadGameAssets). playMusic is a
    // no-op when the theme is already playing, as it is coming back from a
    // run or the options.
    if (auto* audio = registry.ctx().find<AudioManager>(); audio != nullptr)
    {
        audio->playMusic("theme", true);
    }

    auto entity = track(registry.create());
    registry.emplace<Widget>(entity, std::make_unique<TitleWidget>());
}
