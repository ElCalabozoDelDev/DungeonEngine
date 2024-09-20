#include "systems/render_system.hpp"
#include "core/texture_manager.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "world/tile_layer.hpp"
#include <iostream>

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();
  auto view = registry.view<PositionComponent, TextureComponent, SpriteComponent>();

  for (auto entity : view) {
    auto &pos = view.get<PositionComponent>(entity);
    auto &tex = view.get<TextureComponent>(entity);
    auto &spr = view.get<SpriteComponent>(entity);
    TheTextureManager::Instance()->drawFrame(tex.id, pos.position.getX(), pos.position.getY(), spr.spriteWidth, spr.spriteHeight, spr.spriteRow, spr.currentSprite, renderer, 0, 255, SDL_FLIP_NONE);
  }

  TileLayer::render(registry);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}