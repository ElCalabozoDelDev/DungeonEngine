#include <engine/loaders/config_loader.hpp>
#include <engine/plugins/base_plugin.hpp>
#include <engine/systems/render_system.hpp>
#include <engine/systems/transform_system.hpp>
#include <engine/systems/update_animation_system.hpp>
#include <string>

namespace de
{
BasePlugin::BasePlugin(const std::string& configPath)
{
    ConfigLoader::loadConfigFromXML(configPath.c_str(), m_config);
}

void BasePlugin::mount(GameLoop& gameLoop)
{
    gameLoop.addSetupCallback([this](entt::registry& registry)
                              { registry.ctx().emplace<Config>(m_config); });
    gameLoop.addPlugin(m_sdl);
    gameLoop.addPlugin(m_imgui);
    gameLoop.addPlugin(m_widget);
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
}

} // namespace de
