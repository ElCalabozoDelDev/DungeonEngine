#ifndef GAME_PREFABS_HPP
#define GAME_PREFABS_HPP

#include <engine/core/vector_2d.hpp>
#include <engine/spatial/quadtree.hpp>
#include <entt/entt.hpp>
#include <random>

/// How the game's entities are built. Every place that creates a slime or a
/// bat goes through here, so its components and visual constants cannot drift
/// between the Tiled spawn, the fallback spawn and the body segments.
namespace game::prefab
{
/// Sprite sheets are horizontal strips; AnimationComponent walks columns.
struct SpriteStrip
{
    const char* texture;
    int frames;
    float frameSeconds;
};

inline constexpr SpriteStrip SlimeStrip{"slime", 2, 0.2f};
inline constexpr SpriteStrip BatStrip{"bat", 3, 0.1f};

/// Turns `entity` into the player's snake head: one segment centred on
/// `center`, facing right, `size` logical pixels wide, stepping `size` per
/// stride. Components already on the entity (a Tiled object) are replaced.
void makeSnakeHead(entt::registry& registry, entt::entity entity,
                   const de::Vector2D<float>& center, float size);

/// A sprite entity for one body segment. SnakeViewSystem places it.
entt::entity makeSegmentSprite(entt::registry& registry, float size);

/// A bat at a random position inside `room`, flying in a random direction.
entt::entity makeBat(entt::registry& registry, const de::Box<float>& room,
                     std::mt19937& rng);

} // namespace game::prefab

#endif // GAME_PREFABS_HPP
