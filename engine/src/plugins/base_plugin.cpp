#include <engine/plugins/base_plugin.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/systems/transform_system.hpp>
#include <engine/systems/update_animation_system.hpp>

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

    gameLoop.addPlugin(m_sdl);
    gameLoop.addPlugin(m_imgui);
    gameLoop.addPlugin(m_widget);
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
}

} // namespace de
