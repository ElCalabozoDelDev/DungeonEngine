#include <engine/plugins/base_plugin.hpp>
#include <engine/plugins/imgui_plugin.hpp>
#include <engine/plugins/input_plugin.hpp>
#include <engine/plugins/sdl_plugin.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/systems/spatial_sync_system.hpp>
#include <engine/systems/transform_system.hpp>
#include <engine/systems/update_animation_system.hpp>
#include <memory>

namespace de
{
void BasePlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback(
        [this](entt::registry& registry)
        {
            registry.ctx().emplace<Config>(m_config);
            registry.ctx().emplace<AssetPaths>(m_assets);
            // Empty until a scene builds its layers; owned by the registry so
            // it cannot outlive the entities it indexes.
            registry.ctx().emplace<SpatialIndex>();
        });

    // Order matters: SDL creates the window, input drains the event queue,
    // and only then does ImGui start its frame with those events already
    // delivered.
    gameLoop.addPlugin(std::make_unique<SDLPlugin>());
    gameLoop.addPlugin(std::make_unique<InputPlugin>());
    gameLoop.addPlugin(std::make_unique<ImGuiPlugin>());

    // Integration advances by a fixed step so movement does not depend on
    // frame rate; animation and rendering follow the frame.
    gameLoop.addFixedSystem(std::make_shared<TransformSystem>());
    // Immediately after integration: whatever moved must be re-filed before
    // anything queries the index.
    gameLoop.addFixedSystem(std::make_shared<SpatialSyncSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
}

} // namespace de
