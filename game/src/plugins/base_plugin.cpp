#include "plugins/base_plugin.hpp"
#include "loaders/config_loader.hpp"
#include "systems/movement_system.hpp"
#include "systems/render_system.hpp"
#include "systems/transform_system.hpp"
#include "systems/update_animation_system.hpp"
#include <string>

BasePlugin::BasePlugin(const std::string &configPath) {
  ConfigLoader::loadConfigFromXML(configPath.c_str(), m_config);
  m_sdl = SDLPlugin();
  m_imgui = ImGuiPlugin();
  m_widget = WidgetPlugin();
  m_gameXmlPath.path = configPath;
}

void BasePlugin::mount(GameLoop &gameLoop) {

    gameLoop.addSetupCallback([this](entt::registry &registry) {
      registry.ctx().emplace<Config>(m_config);
      registry.ctx().emplace<GameXmlPath>(m_gameXmlPath);
    });
    gameLoop.addPlugin(m_sdl);
    gameLoop.addPlugin(m_imgui);
    gameLoop.addPlugin(m_widget);
    gameLoop.addSystem(std::make_shared<MovementSystem>());
    gameLoop.addSystem(std::make_shared<TransformSystem>());
    gameLoop.addSystem(std::make_shared<UpdateAnimationSystem>());
    gameLoop.addSystemLast(std::make_shared<RenderSystem>());
  }