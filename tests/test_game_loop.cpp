#include <doctest/doctest.h>
#include <engine/core/delta_time.hpp>
#include <engine/core/game_loop.hpp>
#include <engine/core/paused.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace de;

namespace
{
/// Appends its name to a shared log every time it runs.
class RecordingSystem final : public System
{
public:
    RecordingSystem(std::vector<std::string>& log, std::string name)
        : m_log(log), m_name(std::move(name))
    {
    }

    void run(entt::registry& /*registry*/) override { m_log.push_back(m_name); }

private:
    std::vector<std::string>& m_log;
    std::string m_name;
};

/// Pauses the loop the first time it runs, the way a snake dying does.
class PausingSystem final : public System
{
public:
    explicit PausingSystem(int& runs) : m_runs(runs) {}

    void run(entt::registry& registry) override
    {
        ++m_runs;
        registry.ctx().get<Paused>().value = true;
    }

private:
    int& m_runs;
};

/// A loop on the synthetic clock that runs `frames` frames, each `steps`
/// fixed steps long. No SDL window is involved: run() only reads the
/// performance counter when the synthetic clock is off.
void runLoop(GameLoop& loop, int frames, int steps)
{
    loop.setFrameDelta(DeltaTime{}.fixed * static_cast<float>(steps));
    loop.addSetupCallback(
        [](entt::registry& registry)
        {
            registry.ctx().emplace<DeltaTime>();
            registry.ctx().emplace<Paused>();
        });
    loop.addFrameEndCallback(
        [remaining = frames](entt::registry& registry) mutable
        {
            if (--remaining <= 0)
            {
                registry.ctx().get<ControlFlow>() = ControlFlow::Exit;
            }
        });
    REQUIRE(loop.run());
}

} // namespace

TEST_CASE("fixed-last systems run after every fixed system, every step")
{
    std::vector<std::string> log;
    GameLoop loop;
    // Registered first, as BasePlugin registers SpatialSyncSystem before the
    // game mounts its movement systems.
    loop.addFixedSystemLast(std::make_shared<RecordingSystem>(log, "sync"));
    loop.addFixedSystem(std::make_shared<RecordingSystem>(log, "move"));

    runLoop(loop, 1, 2);

    CHECK(log == std::vector<std::string>{"move", "sync", "move", "sync"});
}

TEST_CASE("a step that pauses is the last one in its frame")
{
    // Regression test. Pause was read once per frame, so after the step that
    // ended the game, the rest of a slow frame's steps still ran.
    int runs = 0;
    GameLoop loop;
    loop.addFixedSystem(std::make_shared<PausingSystem>(runs));

    runLoop(loop, 3, 4);

    CHECK(runs == 1);
}
