#ifndef DE_CORE_GAME_LOOP_HPP
#define DE_CORE_GAME_LOOP_HPP

#include <engine/core/delta_time.hpp>
#include <engine/core/hook.hpp>
#include <engine/core/paused.hpp>
#include <engine/core/startup_error.hpp>
#include <engine/plugins/plugin.hpp>
#include <engine/systems/system.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace de
{
enum class ControlFlow
{
    Exit,
    Loop
};

class GameLoop
{
public:
    /// Longest frame the loop will admit. A frame longer than this (dragging
    /// the window, a breakpoint, the machine sleeping) is treated as this
    /// long, so entities step slowly rather than teleporting through walls.
    static constexpr float MaxFrameDelta = 0.25f;

    /// Cap on fixed steps per frame. Without it, a machine that cannot keep up
    /// runs more steps each frame, which makes it slower still.
    static constexpr int MaxFixedStepsPerFrame = 5;

    /// Takes ownership of the plugin, then mounts it.
    ///
    /// Plugins register callbacks that capture `this`, so the loop has to
    /// outlive them. The previous signature took a bare reference, and
    /// `addPlugin(GamePlugin())` compiled happily and left every one of those
    /// callbacks pointing at a destroyed temporary.
    GameLoop& addPlugin(std::unique_ptr<Plugin> plugin)
    {
        Plugin* raw = plugin.get();
        m_plugins.push_back(std::move(plugin));
        raw->mount(*this);
        return *this;
    }

    /// Runs once per frame, with DeltaTime::value.
    GameLoop& addSystem(std::shared_ptr<System> system)
    {
        m_systems.push_back(std::move(system));
        return *this;
    }

    /// Runs zero or more times per frame, always advancing DeltaTime::fixed.
    /// Movement, physics and collision belong here: their results must not
    /// depend on how fast the machine happens to be drawing.
    GameLoop& addFixedSystem(std::shared_ptr<System> system)
    {
        m_fixedSystems.push_back(std::move(system));
        return *this;
    }

    /// Runs in every fixed step, after every addFixedSystem(). Bookkeeping
    /// that must see the step's final positions goes here -- re-filing the
    /// spatial index, for one -- so it does not depend on the order plugins
    /// were mounted in.
    GameLoop& addFixedSystemLast(std::shared_ptr<System> system)
    {
        m_fixedSystemsLast.push_back(std::move(system));
        return *this;
    }

    /// Runs once per frame, after every addSystem(). Rendering goes here.
    GameLoop& addSystemLast(std::shared_ptr<System> system)
    {
        m_systemsLast.push_back(std::move(system));
        return *this;
    }

    /// Replaces the wall clock with a fixed delta per frame, so a run is
    /// reproducible.
    ///
    /// Normally the frame delta comes from the performance counter, so the
    /// number of fixed steps in a frame depends on how fast the machine is.
    /// That is right for playing and useless for measuring: two runs of the
    /// same binary diverge. With DeltaTime::fixed passed here, the accumulator
    /// receives exactly the float it compares against and subtracts, so every
    /// frame runs exactly one fixed step and lands back on zero.
    ///
    /// Zero, the default, keeps the real clock. Nothing else in the loop
    /// changes, so the normal game path is unaffected.
    GameLoop& setFrameDelta(float seconds)
    {
        m_frameDelta = seconds;
        return *this;
    }

    GameLoop& addSetupCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookSetup.connect(std::move(callback));
        return *this;
    }

    GameLoop&
    addFrameBeginCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookFrameBegin.connect(std::move(callback));
        return *this;
    }

    GameLoop& addFrameEndCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookFrameEnd.connect(std::move(callback));
        return *this;
    }

    GameLoop& addTeardownCallback(std::function<void(entt::registry&)> callback)
    {
        m_hookTeardown.connect(std::move(callback), true);
        return *this;
    }

    /// Runs setup, then frames until something asks to exit.
    ///
    /// The loop owns DeltaTime and ControlFlow and publishes both before any
    /// setup callback runs. Setup stops at the first callback that reports a
    /// StartupError: later callbacks are not called, so none of them needs to
    /// check for one. run() then returns false and no frame runs. Teardown
    /// runs either way, so teardown callbacks must cope with a setup that
    /// only got part-way.
    bool run();

private:
    bool isPaused() const;

    entt::registry m_registry;
    ControlFlow m_controlFlow = ControlFlow::Exit;

    /// Zero means the real clock; see setFrameDelta().
    float m_frameDelta = 0.0f;

    std::vector<std::unique_ptr<Plugin>> m_plugins;
    std::vector<std::shared_ptr<System>> m_fixedSystems;
    std::vector<std::shared_ptr<System>> m_fixedSystemsLast;
    std::vector<std::shared_ptr<System>> m_systems;
    std::vector<std::shared_ptr<System>> m_systemsLast;

    Hook<void(entt::registry&)> m_hookSetup;
    Hook<void(entt::registry&)> m_hookFrameBegin;
    Hook<void(entt::registry&)> m_hookFrameEnd;
    Hook<void(entt::registry&)> m_hookTeardown;
};

} // namespace de

#endif // DE_CORE_GAME_LOOP_HPP
