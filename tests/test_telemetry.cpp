#include <algorithm>
#include <doctest/doctest.h>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/components/velocity_component.hpp>
#include <engine/core/delta_time.hpp>
#include <engine/core/game_loop.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/health_component.hpp>
#include <game/components/item_component.hpp>
#include <game/components/player_component.hpp>
#include <game/sim/telemetry_sampler.hpp>
#include <game/sim/telemetry_writer.hpp>
#include <game/state.hpp>
#include <sstream>
#include <string>

using namespace de;

namespace
{
/// Everything the sampler reads, and a ControlFlow it can end the run with.
/// The sampler is driven directly rather than through a game loop, so a step
/// here is one call to run().
struct SamplerWorld
{
    entt::registry registry;
    ControlFlow flow = ControlFlow::Loop;
    entt::entity player;
    /// One uncollected item, so the sampler does not declare the run cleared
    /// on its very first step and stop sampling.
    entt::entity item;

    SamplerWorld()
    {
        registry.ctx().emplace<DeltaTime>(
            DeltaTime{1.0f / 60.0f, 1.0f / 60.0f, 0.0f, 0.0});
        registry.ctx().emplace<GameState>();
        registry.ctx().emplace<ControlFlow&>(flow);

        player = registry.create();
        registry.emplace<PlayerComponent>(player);
        registry.emplace<TransformComponent>(player,
                                             Vector2D<float>(100.0f, 100.0f));
        registry.emplace<DimensionComponent>(player, 16.0f, 16.0f);
        registry.emplace<VelocityComponent>(player);
        registry.emplace<HealthComponent>(player);

        item = registry.create();
        registry.emplace<ItemComponent>(item);
    }

    entt::entity enemy(float x, float y)
    {
        auto entity = registry.create();
        registry.emplace<EnemyComponent>(entity);
        registry.emplace<TransformComponent>(entity, Vector2D<float>(x, y));
        registry.emplace<DimensionComponent>(entity, 16.0f, 16.0f);
        return entity;
    }

    GameState& state() { return registry.ctx().get<GameState>(); }

    void moveTo(float x, float y)
    {
        registry.get<TransformComponent>(player).position =
            Vector2D<float>(x, y);
    }
};

/// A sampler with no policy input of its own: the default command is idle,
/// which is what most of these cases want.
SeekItemsPolicy idlePolicy;

} // namespace

TEST_CASE("a row reports the state after the step, not before it")
{
    SamplerWorld world;
    TelemetrySampler sampler(idlePolicy, 100);

    sampler.run(world.registry);
    world.moveTo(140.0f, 100.0f);
    world.state().itemsCollected = 1;
    world.registry.get<HealthComponent>(world.player).current = 4;
    sampler.run(world.registry);

    const TelemetryRow& row = sampler.lastRow();
    CHECK(row.step == 2);
    CHECK(row.time == doctest::Approx(2.0f / 60.0f));
    CHECK(row.playerX == doctest::Approx(140.0f));
    CHECK(row.itemsCollected == 1);
    CHECK(row.health == 4);

    SUBCASE("distance accumulates from the movement that happened")
    {
        CHECK(row.distance == doctest::Approx(40.0f));
    }

    SUBCASE("a drop in health is counted as damage and as a hit")
    {
        CHECK(row.damageTaken == 1);
        CHECK(sampler.summary().hits == 1);
        CHECK(sampler.summary().firstHitAt == doctest::Approx(2.0f / 60.0f));
    }

    SUBCASE("the pickup is stamped with the step it happened on")
    {
        REQUIRE(sampler.summary().pickupTimes.size() == 1);
        CHECK(sampler.summary().pickupTimes.front() ==
              doctest::Approx(2.0f / 60.0f));
    }
}

TEST_CASE("enemy columns describe the chase, not just proximity")
{
    SamplerWorld world;
    TelemetrySampler sampler(idlePolicy, 100);

    SUBCASE("with no enemies the distance is the -1 sentinel, not zero")
    {
        sampler.run(world.registry);
        CHECK(sampler.lastRow().nearestEnemyDistance == doctest::Approx(-1.0f));
        CHECK(sampler.lastRow().enemiesChasing == 0);
    }

    SUBCASE("only enemies inside chaseRange count as chasing")
    {
        // Default chaseRange is 120. Centres are 40 and 400 apart.
        world.enemy(140.0f, 100.0f);
        world.enemy(500.0f, 100.0f);
        sampler.run(world.registry);

        CHECK(sampler.lastRow().enemiesChasing == 1);
        CHECK(sampler.lastRow().nearestEnemyDistance == doctest::Approx(40.0f));
    }
}

TEST_CASE("the run ends for the right reason")
{
    SUBCASE("collecting the last item clears it")
    {
        SamplerWorld world;
        TelemetrySampler sampler(idlePolicy, 100);

        sampler.run(world.registry);
        CHECK_FALSE(sampler.finished());

        world.registry.destroy(world.item);
        world.state().itemsCollected = 1;
        sampler.run(world.registry);

        CHECK(sampler.finished());
        CHECK(sampler.summary().outcome == "cleared");
        CHECK(world.flow == ControlFlow::Exit);
    }

    SUBCASE("running out of health is a death, and ends the run there")
    {
        // The sampler has to be what stops the loop: it is the only thing
        // between CombatSystem setting gameOver and GamePlugin's next
        // frame-begin clearing it and bouncing to the menu.
        SamplerWorld world;
        TelemetrySampler sampler(idlePolicy, 100);
        world.state().gameOver = true;
        sampler.run(world.registry);

        CHECK(sampler.summary().outcome == "died");
        CHECK(world.flow == ControlFlow::Exit);
    }

    SUBCASE("the step cap stops a run that would otherwise never end")
    {
        SamplerWorld world;
        TelemetrySampler sampler(idlePolicy, 3);

        for (int step = 0; step < 5; ++step)
        {
            sampler.run(world.registry);
        }
        CHECK(sampler.summary().outcome == "step_cap");
        CHECK(sampler.summary().steps == 3);
    }

    SUBCASE("no player means no rows at all")
    {
        SamplerWorld world;
        world.registry.destroy(world.player);
        TelemetrySampler sampler(idlePolicy, 100);
        sampler.run(world.registry);

        CHECK(sampler.lastRow().step == 0);
        CHECK_FALSE(sampler.finished());
    }
}

TEST_CASE("the CSV is strict and its shape matches the header")
{
    std::ostringstream out;
    TelemetryWriter writer(out);
    writer.writeHeader();
    writer.write(TelemetryRow{});

    std::istringstream in(out.str());
    std::string header;
    std::string row;
    REQUIRE(std::getline(in, header));
    REQUIRE(std::getline(in, row));

    const auto fields = [](const std::string& line)
    { return std::count(line.begin(), line.end(), ',') + 1; };

    SUBCASE("every column in the header has a value in the row")
    {
        // Cheap, and it catches every future column added on one side only.
        CHECK(fields(header) == fields(row));
        CHECK(fields(header) == 19);
    }

    SUBCASE("no comment or metadata lines, so any reader can parse it")
    {
        CHECK(out.str().find('#') == std::string::npos);
        CHECK(header.rfind("step,time,", 0) == 0);
    }
}

TEST_CASE("the summary reports what the run actually did")
{
    SimSummary summary;
    summary.outcome = "cleared";
    summary.steps = 254;
    summary.fixedDelta = 1.0f / 60.0f;
    summary.items = 3;
    summary.itemsTotal = 3;
    summary.pickupTimes = {1.35f, 3.0f};
    summary.damageTaken = 1;
    summary.distance = 848.0f;

    const std::string text = TelemetryWriter::summary(summary);

    CHECK(text.find("outcome=cleared") != std::string::npos);
    CHECK(text.find("items=3/3") != std::string::npos);
    CHECK(text.find("steps=254") != std::string::npos);

    SUBCASE("a run with no pickups says so instead of printing nothing")
    {
        SimSummary empty;
        empty.fixedDelta = 1.0f / 60.0f;
        CHECK(TelemetryWriter::summary(empty).find("pickups_at=none") !=
              std::string::npos);
    }

    SUBCASE("never being hit is said in words, not as a bare -1")
    {
        CHECK(text.find("first_hit_at=never") != std::string::npos);
    }
}
