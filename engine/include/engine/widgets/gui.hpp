#ifndef DE_WIDGETS_GUI_HPP
#define DE_WIDGETS_GUI_HPP

#include <entt/entt.hpp>

namespace de::gui
{
/// A piece of immediate-mode GUI, drawn once per frame by RenderSystem.
///
/// State that must survive between frames -- which button has focus, how far
/// a background has scrolled -- lives in the subclass's own members. Widgets
/// used to keep it in React-style hooks matched by call order, which broke
/// the moment a hook was called from inside a branch.
class WidgetComponent
{
public:
    virtual ~WidgetComponent() = default;

    virtual void render(entt::registry& registry) = 0;
};

} // namespace de::gui

#endif // DE_WIDGETS_GUI_HPP
