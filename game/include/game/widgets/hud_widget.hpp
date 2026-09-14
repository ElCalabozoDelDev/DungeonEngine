#ifndef GAME_WIDGETS_HUD_WIDGET_HPP
#define GAME_WIDGETS_HUD_WIDGET_HPP

#include <engine/widgets/gui.hpp>
#include <game/ui/menu.hpp>

/// Score HUD plus pause and game-over overlays.
class HudWidget : public de::gui::WidgetComponent
{
public:
    void render(entt::registry& registry) override;

private:
    // One focus per overlay: moving to QUIT on Game Over must not leave QUIT
    // focused the next time the game is paused.
    game::ui::MenuNav m_pauseNav{2, game::ui::MenuNav::Ends::Clamp};
    game::ui::MenuNav m_gameOverNav{2, game::ui::MenuNav::Ends::Clamp};
};

#endif // GAME_WIDGETS_HUD_WIDGET_HPP
