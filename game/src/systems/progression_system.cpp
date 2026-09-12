#include <engine/audio/audio_manager.hpp>
#include <engine/components/dimension_component.hpp>
#include <engine/components/transform_component.hpp>
#include <engine/spatial/quadtree.hpp>
#include <engine/spatial/spatial_index.hpp>
#include <game/components/objective_component.hpp>
#include <game/components/player_component.hpp>
#include <game/components/stairs_component.hpp>
#include <game/run/run_config.hpp>
#include <game/state.hpp>
#include <game/systems/progression_system.hpp>
#include <vector>

using namespace de;

namespace
{
Box<float> boxOf(entt::registry& registry, entt::entity entity)
{
    const auto& transform = registry.get<TransformComponent>(entity);
    const auto& dimension = registry.get<DimensionComponent>(entity);
    return Box<float>(transform.position.getX(), transform.position.getY(),
                      dimension.width, dimension.height);
}
} // namespace

void ProgressionSystem::run(entt::registry& registry)
{
    auto* state = registry.ctx().find<GameState>();
    auto* config = registry.ctx().find<RunConfig>();
    auto* spatial = registry.ctx().find<SpatialIndex>();
    auto* audio = registry.ctx().find<AudioManager>();
    if (state == nullptr || config == nullptr)
    {
        return;
    }

    auto players =
        registry
            .view<PlayerComponent, TransformComponent, DimensionComponent>();
    for (auto player : players)
    {
        const Box<float> playerBox = boxOf(registry, player);
        const auto& pos = registry.get<TransformComponent>(player).position;
        const int col = static_cast<int>(
            (pos.getX() + config->tileSize * 0.5f) / config->tileSize);
        const int row = static_cast<int>(
            (pos.getY() + config->tileSize * 0.5f) / config->tileSize);

        // Win: back on floor 1 entrance tile with the objective.
        if (state->hasObjective && state->currentFloor == 1 &&
            state->entranceRecorded && col == state->entranceColumn &&
            row == state->entranceRow)
        {
            state->victory = true;
        }

        std::vector<entt::entity> takenObjectives;
        auto objectives = registry.view<ObjectiveComponent, TransformComponent,
                                        DimensionComponent>();
        for (auto objective : objectives)
        {
            if (playerBox.intersects(boxOf(registry, objective)))
            {
                takenObjectives.push_back(objective);
            }
        }
        for (auto objective : takenObjectives)
        {
            state->hasObjective = true;
            if (audio != nullptr)
            {
                audio->playSound("pickup");
            }
            if (spatial != nullptr)
            {
                spatial->remove(Layer::Object, objective);
            }
            registry.destroy(objective);
        }

        auto stairs = registry.view<StairsComponent, TransformComponent,
                                    DimensionComponent>();
        for (auto stair : stairs)
        {
            if (!playerBox.intersects(boxOf(registry, stair)))
            {
                continue;
            }
            if (state->pendingFloorChange != 0)
            {
                break;
            }
            const auto direction =
                registry.get<StairsComponent>(stair).direction;
            if (direction == StairsDirection::Down &&
                state->currentFloor < config->floorsPerRun)
            {
                state->pendingFloorChange = state->currentFloor + 1;
            }
            else if (direction == StairsDirection::Up &&
                     state->currentFloor > 1)
            {
                state->pendingFloorChange = state->currentFloor - 1;
            }
            break;
        }
    }
}
