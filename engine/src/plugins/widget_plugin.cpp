#include <engine/core/game_loop.hpp>
#include <engine/plugins/widget_plugin.hpp>
#include <engine/widgets/gui.hpp>

namespace de
{
void WidgetPlugin::mount(GameLoop& gameLoop)
{
    // Nothing to mount yet: RenderSystem already walks every widget component
    // in the registry, so a game adds one by creating an entity holding its
    // own gui::WidgetComponent. Spawning a concrete widget here would mean the
    // engine depending on a game type.
    //
    // Wiring this plugin up to own widget lifetime is Phase 5 of the
    // restructuring.
    (void)gameLoop;
}

} // namespace de
