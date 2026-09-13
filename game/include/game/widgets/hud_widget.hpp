#ifndef GAME_WIDGETS_HUD_WIDGET_HPP
#define GAME_WIDGETS_HUD_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <entt/entt.hpp>

/// Score HUD plus pause and game-over overlays.
class HudWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry, de::gui::Hooks& h) override;
};

#endif // GAME_WIDGETS_HUD_WIDGET_HPP
