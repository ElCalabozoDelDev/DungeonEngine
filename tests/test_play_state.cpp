#include <doctest/doctest.h>
#include <engine/core/paused.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <game/play_state.hpp>
#include <game/state.hpp>
#include <game/ui/bitmap_font.hpp>
#include <game/ui/ui_skin.hpp>

using namespace de;

TEST_CASE("setPlayState syncs Paused and resetRunPresentation clears grade")
{
    entt::registry registry;
    registry.ctx().emplace<Paused>();
    registry.ctx().emplace<GameState>();
    registry.ctx().emplace<WorldColorGrade>();

    setPlayState(registry, PlayState::GameOver);
    CHECK(registry.ctx().get<GameState>().playState == PlayState::GameOver);
    CHECK(registry.ctx().get<Paused>().value);

    registry.ctx().get<WorldColorGrade>().colorAmount = 0.0f;
    resetRunPresentation(registry);
    CHECK(registry.ctx().get<GameState>().playState == PlayState::Playing);
    CHECK_FALSE(registry.ctx().get<Paused>().value);
    CHECK(registry.ctx().get<WorldColorGrade>().colorAmount ==
          doctest::Approx(1.0f));
}

TEST_CASE("bitmap font measures scales used by UI")
{
    // Headless: only glyph metrics from the .fnt — no SDL texture.
    game::ui::BitmapFont font;
    // load needs a renderer; pin layout constants instead when no window.
    CHECK(game::ui::kFontScore == doctest::Approx(0.25f));
    CHECK(game::ui::kFontButton == doctest::Approx(0.25f));
    CHECK(game::ui::kFontPanelTitle == doctest::Approx(0.5f));
    CHECK(game::ui::kFontTitle == doctest::Approx(1.0f));
    CHECK(game::ui::kPanelBorder == doctest::Approx(5.0f));
    CHECK(game::ui::kTitleDungeonCenter.x == doctest::Approx(160.0f));
    CHECK(game::ui::kTitleDungeonCenter.y == doctest::Approx(25.0f));
    (void)font;
}
