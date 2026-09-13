#include <engine/audio/audio_manager.hpp>
#include <engine/core/asset_paths.hpp>
#include <engine/core/paused.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/widgets/widget.hpp>
#include <game/scene/title_scene.hpp>
#include <game/state.hpp>
#include <game/widgets/title_widget.hpp>
#include <memory>

using namespace de;

void TitleScene::onEnter(entt::registry& registry)
{
    registry.ctx().get<SpatialIndex>().clear();
    registry.ctx().get<Paused>().value = false;

    if (!registry.ctx().contains<AudioSettings>())
    {
        registry.ctx().emplace<AudioSettings>();
    }

    const auto& assets = registry.ctx().get<AssetPaths>();
    if (auto* textures = registry.ctx().find<TextureCache>();
        textures != nullptr)
    {
        textures->load(
            "bg-pattern",
            assets.resolve("images/background-pattern.png").string());
    }

    if (auto* audio = registry.ctx().find<AudioManager>(); audio != nullptr)
    {
        audio->loadSound("ui", assets.resolve("Audio/ui.wav").string());
        const auto& settings = registry.ctx().get<AudioSettings>();
        audio->setMusicVolume(settings.musicPercent);
        audio->setSfxVolume(settings.sfxPercent);
        audio->stopMusic();
    }

    auto entity = registry.create();
    registry.emplace<Widget>(entity, std::make_unique<TitleWidget>());
    m_entities.push_back(entity);
}

void TitleScene::onUpdate(entt::registry& registry) {}

void TitleScene::onExit(entt::registry& registry)
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
