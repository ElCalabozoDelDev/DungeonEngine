#include <engine/audio/audio_manager.hpp>
#include <engine/core/asset_paths.hpp>
#include <engine/graphics/texture_cache.hpp>
#include <game/assets.hpp>
#include <game/state.hpp>

using namespace de;

namespace game
{
void loadGameAssets(entt::registry& registry)
{
    const auto& assets = registry.ctx().get<AssetPaths>();

    auto& textures = registry.ctx().get<TextureCache>();
    // Horizontal strips: slime 2x20, bat 3x20 (see prefab::SlimeStrip).
    textures.load("slime", assets.resolve("images/slime.png").string());
    textures.load("bat", assets.resolve("images/bat.png").string());
    textures.load("bg-pattern",
                  assets.resolve("images/background-pattern.png").string());

    // Audio is optional: the game runs muted when the mixer did not open.
    if (auto* audio = registry.ctx().find<AudioManager>(); audio != nullptr)
    {
        audio->loadSound("bounce", assets.resolve("Audio/bounce.wav").string());
        audio->loadSound("collect",
                         assets.resolve("Audio/collect.wav").string());
        audio->loadSound("ui", assets.resolve("Audio/ui.wav").string());
        audio->loadMusic("theme", assets.resolve("Audio/theme.ogg").string());

        const auto& settings = registry.ctx().get<AudioSettings>();
        audio->setMusicVolume(settings.musicPercent);
        audio->setSfxVolume(settings.sfxPercent);
    }
}

} // namespace game
