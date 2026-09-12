#ifndef GAME_PROC_LEVEL_ASSEMBLER_HPP
#define GAME_PROC_LEVEL_ASSEMBLER_HPP

#include <engine/core/asset_paths.hpp>
#include <entt/entt.hpp>
#include <expected>
#include <game/proc/floor_blueprint.hpp>
#include <string>
#include <vector>

/// Turns a FloorBlueprint into the same ECS shape TMXLoader produces so the
/// rest of the scene pipeline (tagging, spatial index, render) stays unchanged.
class LevelAssembler
{
public:
    explicit LevelAssembler(std::vector<entt::entity>* entities)
        : m_entities(entities)
    {
    }

    std::expected<void, std::string> assemble(entt::registry& registry,
                                              const FloorBlueprint& floor,
                                              const de::AssetPaths& assets);

    int mapPixelWidth() const { return m_mapPixelWidth; }
    int mapPixelHeight() const { return m_mapPixelHeight; }

private:
    std::vector<entt::entity>* m_entities = nullptr;
    int m_mapPixelWidth = 0;
    int m_mapPixelHeight = 0;
};

#endif // GAME_PROC_LEVEL_ASSEMBLER_HPP
