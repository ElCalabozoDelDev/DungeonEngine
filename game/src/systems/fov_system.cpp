#include <engine/components/transform_component.hpp>
#include <engine/spatial/visibility_map.hpp>
#include <game/components/player_component.hpp>
#include <game/run/run_config.hpp>
#include <game/systems/fov_system.hpp>
#include <libtcod/fov.h>
#include <libtcod/fov_types.h>

using namespace de;

void FovSystem::run(entt::registry& registry)
{
    auto* visibility = registry.ctx().find<VisibilityMap>();
    auto* config = registry.ctx().find<RunConfig>();
    if (visibility == nullptr || !visibility->enabled || config == nullptr ||
        visibility->opaque.size() != visibility->visible.size())
    {
        return;
    }

    auto players = registry.view<PlayerComponent, TransformComponent>();
    if (players.begin() == players.end())
    {
        return;
    }

    const auto player = *players.begin();
    const auto& pos = players.get<TransformComponent>(player).position;
    const int px = static_cast<int>((pos.getX() + visibility->tileSize * 0.5f) /
                                    visibility->tileSize);
    const int py = static_cast<int>((pos.getY() + visibility->tileSize * 0.5f) /
                                    visibility->tileSize);

    TCOD_Map* map = TCOD_map_new(visibility->columns, visibility->rows);
    if (map == nullptr)
    {
        return;
    }

    for (int row = 0; row < visibility->rows; ++row)
    {
        for (int col = 0; col < visibility->columns; ++col)
        {
            const bool blocks = visibility->opaque[static_cast<std::size_t>(
                                    visibility->index(col, row))] != 0;
            TCOD_map_set_properties(map, col, row, !blocks, !blocks);
        }
    }

    if (visibility->inBounds(px, py))
    {
        // libtcod recursive shadowcasting — established algo, not invented
        // here.
        TCOD_map_compute_fov(map, px, py, config->fovRadiusTiles, true,
                             FOV_SHADOW);
    }

    for (int row = 0; row < visibility->rows; ++row)
    {
        for (int col = 0; col < visibility->columns; ++col)
        {
            const std::size_t i =
                static_cast<std::size_t>(visibility->index(col, row));
            const bool inFov = visibility->inBounds(px, py) &&
                               TCOD_map_is_in_fov(map, col, row);
            visibility->visible[i] = inFov ? 1 : 0;
            if (inFov)
            {
                visibility->explored[i] = 1;
            }
        }
    }

    TCOD_map_delete(map);
}
