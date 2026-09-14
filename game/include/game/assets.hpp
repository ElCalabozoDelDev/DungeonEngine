#ifndef GAME_ASSETS_HPP
#define GAME_ASSETS_HPP

#include <entt/entt.hpp>

namespace game
{
/// Loads the textures, sounds and music every scene shares, and applies the
/// current AudioSettings volumes. Called once at startup, so scenes only use
/// assets and never (re)load them.
///
/// Includes the menu chrome and font (game::ui::loadUiAssets). Level tilesets
/// are not included: TiledLoader loads those with the map.
void loadGameAssets(entt::registry& registry);

} // namespace game

#endif // GAME_ASSETS_HPP
