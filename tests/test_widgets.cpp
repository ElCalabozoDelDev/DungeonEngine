#include <SDL.h>
#include <array>
#include <doctest/doctest.h>
#include <engine/components/camera_bounds_component.hpp>
#include <engine/components/camera_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/core/debug.hpp>
#include <engine/core/paused.hpp>
#include <engine/input/action_map.hpp>
#include <engine/input/input_state.hpp>
#include <engine/scene/scene_system.hpp>
#include <engine/widgets/gui.hpp>
#include <game/play_state.hpp>
#include <game/state.hpp>
#include <game/ui/menu.hpp>
#include <game/widgets/hud_widget.hpp>
#include <imgui.h>
#include <initializer_list>

using namespace de;

namespace
{
/// A headless Dear ImGui context: no window and no renderer backend, just
/// enough for widgets to run NewFrame/EndFrame.
struct ImGuiFrameContext
{
    ImGuiFrameContext()
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(320.0f, 180.0f);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    ~ImGuiFrameContext() { ImGui::DestroyContext(); }

    ImGuiFrameContext(const ImGuiFrameContext&) = delete;
    ImGuiFrameContext& operator=(const ImGuiFrameContext&) = delete;
};

/// Runs one GUI frame with exactly `held` keys down.
void frame(entt::registry& registry, gui::WidgetComponent& widget,
           std::initializer_list<SDL_Scancode> held = {})
{
    std::array<Uint8, SDL_NUM_SCANCODES> keys{};
    for (auto key : held)
    {
        keys[static_cast<std::size_t>(key)] = 1;
    }
    auto& input = registry.ctx().get<InputState>();
    input.beginFrame();
    input.setKeyboard(keys.data(), SDL_NUM_SCANCODES);

    ImGui::NewFrame();
    widget.render(registry);
    ImGui::EndFrame();
}

/// Draws a widget through the entity editor's callback inside a frame.
template <typename Component>
void editorWidget(entt::registry& registry, entt::entity entity)
{
    ImGui::NewFrame();
    ImGui::Begin("Inspector");
    MM::ComponentEditorWidget<Component>(registry, entity);
    ImGui::End();
    ImGui::EndFrame();
}

} // namespace

TEST_CASE("the camera inspector copes with a camera that has no bounds")
{
    // Regression test. The widget asked for CameraBoundsComponent
    // unconditionally; Dungeon Slime's fixed camera has none, so opening the
    // inspector on it asserted.
    ImGuiFrameContext imgui;
    entt::registry registry;

    auto camera = registry.create();
    registry.emplace<CameraComponent>(camera);
    editorWidget<CameraComponent>(registry, camera);

    registry.emplace<TransformComponent>(camera, Vector2D<float>(1.0f, 2.0f));
    editorWidget<CameraComponent>(registry, camera);

    registry.emplace<CameraBoundsComponent>(camera, 640, 360);
    editorWidget<CameraComponent>(registry, camera);

    CHECK(registry.get<TransformComponent>(camera).position.getX() ==
          doctest::Approx(1.0f));
}

TEST_CASE("the pause menu does not inherit the game over menu's focus")
{
    // Regression test. Both overlays once declared their focus with a hook
    // inside their own branch, so they shared one slot: moving to QUIT on
    // Game Over left QUIT focused on the next pause, and Enter quit to the
    // title instead of resuming. Each overlay now has its own MenuNav.
    ImGuiFrameContext imgui;
    entt::registry registry;
    registry.ctx().emplace<Paused>();
    registry.ctx().emplace<GameState>();
    registry.ctx().emplace<InputState>();
    auto& actions = registry.ctx().emplace<ActionMap>();
    actions.bind("move_left", SDL_SCANCODE_LEFT);
    actions.bind("move_right", SDL_SCANCODE_RIGHT);
    actions.bind("confirm", SDL_SCANCODE_RETURN);
    SceneSystem scenes;
    registry.ctx().emplace<SceneSystem&>(scenes);

    HudWidget hud;

    setPlayState(registry, PlayState::GameOver);
    frame(registry, hud);
    frame(registry, hud, {SDL_SCANCODE_RIGHT}); // focus QUIT on Game Over
    frame(registry, hud);

    setPlayState(registry, PlayState::Paused);
    frame(registry, hud);
    frame(registry, hud, {SDL_SCANCODE_RETURN}); // RESUME is focused

    CHECK(registry.ctx().get<GameState>().playState == PlayState::Playing);
}

TEST_CASE("MenuNav moves focus with the actions it is given")
{
    entt::registry registry;
    registry.ctx().emplace<InputState>();
    auto& actions = registry.ctx().emplace<ActionMap>();
    actions.bind("move_up", SDL_SCANCODE_UP);
    actions.bind("move_down", SDL_SCANCODE_DOWN);

    const auto press = [&registry](std::initializer_list<SDL_Scancode> held)
    {
        std::array<Uint8, SDL_NUM_SCANCODES> keys{};
        for (auto key : held)
        {
            keys[static_cast<std::size_t>(key)] = 1;
        }
        auto& input = registry.ctx().get<InputState>();
        input.beginFrame();
        input.setKeyboard(keys.data(), SDL_NUM_SCANCODES);
    };
    const auto step = [&](game::ui::MenuNav& nav, SDL_Scancode key)
    {
        press({key});
        nav.update(registry, {"move_up"}, {"move_down"});
        press({}); // release, so the next press is a new one
        nav.update(registry, {"move_up"}, {"move_down"});
    };

    SUBCASE("clamped at both ends")
    {
        game::ui::MenuNav nav(2, game::ui::MenuNav::Ends::Clamp);
        CHECK(nav.focus() == 0);
        step(nav, SDL_SCANCODE_UP);
        CHECK(nav.focus() == 0);
        step(nav, SDL_SCANCODE_DOWN);
        CHECK(nav.focused(1));
        step(nav, SDL_SCANCODE_DOWN);
        CHECK(nav.focus() == 1);
    }

    SUBCASE("wrapping round")
    {
        game::ui::MenuNav nav(3, game::ui::MenuNav::Ends::Wrap);
        step(nav, SDL_SCANCODE_UP);
        CHECK(nav.focus() == 2);
        step(nav, SDL_SCANCODE_DOWN);
        CHECK(nav.focus() == 0);
    }

    SUBCASE("a held key moves once, not every frame")
    {
        game::ui::MenuNav nav(3, game::ui::MenuNav::Ends::Wrap);
        press({SDL_SCANCODE_DOWN});
        nav.update(registry, {"move_up"}, {"move_down"});
        press({SDL_SCANCODE_DOWN});
        nav.update(registry, {"move_up"}, {"move_down"});
        CHECK(nav.focus() == 1);
    }
}
