#include <engine/plugins/audio_plugin.hpp>
#include <engine/plugins/base_plugin.hpp>
#include <engine/plugins/imgui_plugin.hpp>
#include <engine/plugins/input_plugin.hpp>
#include <engine/plugins/sdl_plugin.hpp>
#include <engine/scene/scene_system.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/debug_system.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/systems/spatial_sync_system.hpp>
#include <engine/systems/update_animation_system.hpp>
#include <memory>

namespace de
{
void BasePlugin::mount(GameLoop& gameLoop)
{
    // Scene switching and the entity inspector are part of every game. The
    // loop owns the systems; the context holds references so scenes, widgets
    // and games can reach them.
    auto scenes = std::make_shared<SceneSystem>();
    auto debug = std::make_shared<DebugSystem>();

    gameLoop.addSetupCallback(
        [this, scenes, debug](entt::registry& registry)
        {
            registry.ctx().emplace<Config>(m_config);
            registry.ctx().emplace<AssetPaths>(m_assets);
            // Empty until a scene builds its layers; owned by the registry so
            // it cannot outlive the entities it indexes.
            registry.ctx().emplace<SpatialIndex>();
            registry.ctx().emplace<SceneSystem&>(*scenes);
            registry.ctx().emplace<DebugSystem&>(*debug);
        });

    // Order matters: SDL creates the window, input drains the event queue,
    // and only then does ImGui start its frame with those events already
    // delivered.
    gameLoop.addPlugin(std::make_unique<SDLPlugin>());
    gameLoop.addPlugin(std::make_unique<InputPlugin>());
    gameLoop.addPlugin(std::make_unique<AudioPlugin>());
    gameLoop.addPlugin(std::make_unique<ImGuiPlugin>());

    // Movement is the game's to register: TransformSystem, CameraSystem and
    // CollisionSystem are opt-in, since a game with its own movement rules
    // (Dungeon Slime's grid snake) has no use for them.
    //
    // Whatever moved must be re-filed before anything queries the index. Last
    // in the step: the game's movement systems are mounted after this plugin.
    gameLoop.addFixedSystemLast(std::make_shared<SpatialSyncSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystem(scenes);
    gameLoop.addSystem(debug);

    auto render = std::make_shared<RenderSystem>();
    gameLoop.addSystemLast(render);
    // Teardown runs newest first, so this runs before the SDL plugin's, while
    // the renderer the grade pass's textures belong to still exists.
    gameLoop.addTeardownCallback([render](entt::registry& /*registry*/)
                                 { render->releaseGpuResources(); });
}

} // namespace de
