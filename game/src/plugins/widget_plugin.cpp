#include "plugins/widget_plugin.hpp"

#include "core/game_loop.hpp"
#include "widgets/bottom_gui_widget.hpp"

#include "widgets/gui.hpp"
#include "widgets/right_gui_widget.hpp"
#include <entt/entity/fwd.hpp>

void WidgetPlugin::mount(GameLoop &gameLoop)
{
    gameLoop.addSetupCallback([](entt::registry &registry) {
        entt::entity bgEntity = registry.create();
        registry.emplace<std::unique_ptr<gui::WidgetComponent>>(bgEntity, std::make_unique<BottomGuiWidget>());
        
        entt::entity rgEntity = registry.create();
        registry.emplace<std::unique_ptr<gui::WidgetComponent>>(rgEntity, std::make_unique<RightGuiWidget>());
    });
}