#include <doctest/doctest.h>
#include <game/components/attack_component.hpp>
#include <game/components/enemy_component.hpp>
#include <game/components/health_component.hpp>
#include <game/components/item_component.hpp>
#include <game/components/speed_component.hpp>
#include <game/run/run_config.hpp>

/// The balance numbers live in two places: the component defaults below and
/// docs/design/50-balance/tablas.md. Nothing in the .tmx can override them --
/// tagObjectsByType default-constructs every component -- so the defaults are
/// the whole tuning surface, and a table in a document is only a copy.
///
/// This test is what keeps the copy honest: change a number and it goes red,
/// naming the document you also have to change. See docs/design/90-decisiones/
/// 0001-numeros-en-cpp.md for why the numbers were not moved to game.xml.
TEST_CASE("the balance values match the design document")
{
    SUBCASE("player")
    {
        CHECK(SpeedComponent{}.value == doctest::Approx(200.0f));
        CHECK(HealthComponent{}.current == 5);
        CHECK(HealthComponent{}.max == 5);
        CHECK(HealthComponent{}.invulnerabilityFor == doctest::Approx(0.0f));
    }

    SUBCASE("enemy")
    {
        CHECK(EnemyComponent{}.chaseRange == doctest::Approx(120.0f));
        CHECK(EnemyComponent{}.speed == doctest::Approx(60.0f));
        CHECK(EnemyComponent{}.contactDamage == 1);
        CHECK(EnemyComponent{}.maxHealth == 3);
    }

    SUBCASE("item") { CHECK(ItemComponent{}.value == 1); }

    SUBCASE("attack architecture defaults")
    {
        CHECK(AttackComponent{}.damage == 1);
        CHECK(AttackComponent{}.range == doctest::Approx(24.0f));
        CHECK(AttackComponent{}.cooldownSeconds == doctest::Approx(0.35f));
    }

    SUBCASE("run config defaults stay tunable")
    {
        CHECK(RunConfig{}.floorsPerRun == 3);
        CHECK(RunConfig{}.objectiveFloor == 3);
        CHECK(RunConfig{}.fovRadiusTiles == 8);
    }
}

/// The ratios are the design; the individual values are not. A change that
/// keeps both speeds "reasonable" but moves this ratio has changed PILAR-01
/// (dodge, don't fight) without anyone noticing, which is exactly the kind of
/// drift a number-by-number check misses.
TEST_CASE("the player outruns an enemy by the ratio the pillars assume")
{
    const float ratio = SpeedComponent{}.value / EnemyComponent{}.speed;
    CHECK(ratio == doctest::Approx(3.333f).epsilon(0.01));

    SUBCASE("a hit buys more distance than an enemy's chase range")
    {
        // One second of invulnerability at the speed difference. It is longer
        // than chaseRange, so taking a hit is effectively a free escape --
        // noted in tablas.md because it is almost certainly unintentional.
        const float lead =
            1.0f * (SpeedComponent{}.value - EnemyComponent{}.speed);
        CHECK(lead > EnemyComponent{}.chaseRange);
    }
}
