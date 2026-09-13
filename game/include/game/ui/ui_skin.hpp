#ifndef GAME_UI_UI_SKIN_HPP
#define GAME_UI_UI_SKIN_HPP

#include <entt/entt.hpp>
#include <game/ui/bitmap_font.hpp>
#include <imgui.h>
#include <string_view>

namespace game::ui
{
inline constexpr const char* kPanel = "ui-panel";
inline constexpr const char* kButtonUp = "ui-button-up";
inline constexpr const char* kButtonFocus0 = "ui-button-focus-0";
inline constexpr const char* kButtonFocus1 = "ui-button-focus-1";
inline constexpr const char* kSliderOff = "ui-slider-off";
inline constexpr const char* kSliderMid = "ui-slider-mid";
inline constexpr const char* kSliderMax = "ui-slider-max";

constexpr float kCanvasW = 320.0f;
constexpr float kCanvasH = 180.0f;
constexpr float kButtonH = 14.0f;
constexpr float kButtonMinW = 65.0f;
/// panel-background.png is 15×15 with a 5×5 α=175 centre (Gum 1/3 nine-slice).
constexpr float kPanelBorder = 5.0f;
constexpr float kPausePanelW = 264.0f;
constexpr float kPausePanelH = 70.0f;
constexpr float kSliderPanelW = 264.0f;
constexpr float kSliderPanelH = 55.0f;

constexpr ImU32 kClearColor = IM_COL32(32, 40, 78, 255);
constexpr ImU32 kPatternTint = IM_COL32(255, 255, 255, 128);
constexpr float kPatternScrollSpeed = 50.0f;

/// TitleScene screen positions ÷ present scale (×4).
constexpr ImVec2 kTitleDungeonCenter{160.0f, 25.0f};
constexpr ImVec2 kTitleSlimeCenter{189.25f, 51.75f};
constexpr ImVec2 kTitleShadowOffset{2.5f, 2.5f};
constexpr float kTitleButtonMarginX = 50.0f;
constexpr float kTitleButtonMarginY = 12.0f;

constexpr float kOptionsLabelX = 10.0f;
constexpr float kOptionsLabelY = 10.0f;
constexpr float kOptionsMusicY = 30.0f;
constexpr float kOptionsSfxY = 93.0f;
constexpr float kOptionsBackMarginX = 28.0f;
constexpr float kOptionsBackMarginY = 10.0f;

constexpr float kScoreX = 20.0f;
constexpr float kScoreY = 5.0f;
constexpr float kPauseButtonMargin = 9.0f;
constexpr float kPauseTitleInset = 10.0f;

/// Gum FontScale values on the AngelCode .fnt (size 35).
constexpr float kFontScore = 0.25f;
constexpr float kFontButton = 0.25f;
constexpr float kFontPanelTitle = 0.5f;
/// Title: native BMFont texels in logical space; present nearest ×4.
constexpr float kFontTitle = 1.0f;

constexpr ImU32 kButtonTextColor = IM_COL32(70, 86, 130, 255);
constexpr ImU32 kTitleShadow = IM_COL32(0, 0, 0, 128);

void ensureLoaded(entt::registry& registry);
BitmapFont* font(entt::registry& registry);

int focusFrame(double elapsedSeconds);

void drawNineSlice(ImDrawList* draw, ImTextureID tex, ImVec2 p0, ImVec2 p1,
                   float texW, float texH, float border = kPanelBorder,
                   ImU32 tint = IM_COL32_WHITE);

void drawPanel(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
               ImVec2 p1, ImU32 tint = IM_COL32_WHITE);

ImVec2 buttonSizeForLabel(entt::registry& registry, std::string_view label);

void drawButton(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
                ImVec2 size, bool focused, double elapsedSeconds);

void drawOptionsSlider(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
                       std::string_view label, float value01, bool focused);

void drawCenteredText(entt::registry& registry, ImDrawList* draw, ImVec2 p0,
                      ImVec2 p1, std::string_view text, float fontScale,
                      ImU32 color);

void drawText(entt::registry& registry, ImDrawList* draw, ImVec2 pos,
              std::string_view text, float fontScale, ImU32 color);

bool hitButton(const char* id, ImVec2 p0, ImVec2 size);

} // namespace game::ui

#endif // GAME_UI_UI_SKIN_HPP
