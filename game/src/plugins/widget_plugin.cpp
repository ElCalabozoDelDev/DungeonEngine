#include "plugins/widget_plugin.hpp"

#include "core/game_loop.hpp"
#include "widgets/counter_widget.hpp"
#include "widgets/gui.hpp"

void WidgetPlugin::mount(GameLoop &gameLoop)
{
    // gameLoop.addSetupCallback([](entt::registry &registry) {
    //     entt::entity entity = registry.create();
    //     registry.emplace<std::unique_ptr<gui::WidgetComponent>>(entity, std::make_unique<CounterWidget>());
    // });
}