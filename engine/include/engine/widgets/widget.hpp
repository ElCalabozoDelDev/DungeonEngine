#ifndef DE_WIDGETS_WIDGET_HPP
#define DE_WIDGETS_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <memory>

namespace de
{
/// A GUI widget attached to an entity.
///
/// A component of its own rather than a bare std::unique_ptr<WidgetComponent>:
/// library types make poor component keys -- an entity can only ever hold one,
/// and the inspector shows it as an unreadable mangled name.
struct Widget
{
    std::unique_ptr<gui::WidgetComponent> widget;
};

} // namespace de

#endif // DE_WIDGETS_WIDGET_HPP
