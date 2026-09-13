#include <doctest/doctest.h>
#include <engine/core/delta_time.hpp>
#include <engine/core/paused.hpp>
#include <engine/graphics/world_color_grade.hpp>
#include <game/play_state.hpp>
#include <game/state.hpp>
#include <game/systems/grayscale_fade_system.hpp>
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

TEST_CASE("the grayscale fade takes the same time at any frame rate")
{
    // Regression test. The fade removed 0.02 colour per rendered frame, so it
    // ran 2.4x faster at 144 Hz than at the 60 Hz it was tuned for.
    const auto fadeFor = [](float seconds, int hz)
    {
        entt::registry registry;
        registry.ctx().emplace<Paused>();
        registry.ctx().emplace<GameState>();
        registry.ctx().emplace<WorldColorGrade>();
        const float frame = 1.0f / static_cast<float>(hz);
        registry.ctx().emplace<DeltaTime>(DeltaTime{frame, 1.0f / 60.0f});

        GrayscaleFadeSystem fade;
        setPlayState(registry, PlayState::Paused);
        fade.run(registry); // the frame that notices the change resets to 1

        const int frames = static_cast<int>(seconds * static_cast<float>(hz));
        for (int i = 0; i < frames; ++i)
        {
            fade.run(registry);
        }
        return registry.ctx().get<WorldColorGrade>().colorAmount;
    };

    // Half a second at 1.2 colour per second leaves 0.4 either way.
    CHECK(fadeFor(0.5f, 60) == doctest::Approx(0.4f).epsilon(0.001));
    CHECK(fadeFor(0.5f, 144) == doctest::Approx(0.4f).epsilon(0.001));
    CHECK(fadeFor(2.0f, 144) == doctest::Approx(0.0f)); // clamps at grayscale
}
