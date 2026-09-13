#include <doctest/doctest.h>
#include <engine/loaders/config.hpp>
#include <game/components/bat_component.hpp>
#include <game/components/snake_component.hpp>

TEST_CASE("the balance values match the design document")
{
    SUBCASE("snake movement")
    {
        CHECK(SnakeComponent::movementInterval == doctest::Approx(0.2f));
        CHECK(SnakeComponent::scorePerBat == 100);
        SnakeComponent snake;
        CHECK(snake.stride == doctest::Approx(20.0f));
    }

    SUBCASE("bat movement")
    {
        BatComponent bat;
        CHECK(bat.speed == doctest::Approx(75.0f));
    }

    SUBCASE("logical render size")
    {
        de::Config config;
        CHECK(config.cameraWidth == doctest::Approx(320.0f));
        CHECK(config.cameraHeight == doctest::Approx(180.0f));
        CHECK(config.screenWidth == 1280);
        CHECK(config.screenHeight == 720);
        CHECK(config.zoomLevel == doctest::Approx(1.0f));
    }
}
