#include "systems/render_system.hpp"
#include "components/position_component.hpp"
#include "components/sprite_component.hpp"
#include "components/texture_component.hpp"
#include "components/animation_component.hpp"
#include "core/texture_manager.hpp"

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();

  // Obtener la vista de entidades que tienen tanto PositionComponent como
  // TextureComponent
  auto view = registry.view<PositionComponent, TextureComponent, SpriteComponent>();

  for (auto entity : view) {
    auto &pos = view.get<PositionComponent>(entity);
    auto &tex = view.get<TextureComponent>(entity);
    auto &spr = view.get<SpriteComponent>(entity);

    TheTextureManager::Instance()->drawFrame(registry, tex.texture, pos.position.getX(), pos.position.getY(), spr.spriteWidth, spr.spriteHeight, spr.spriteRow, spr.currentSprite, 0, 255, SDL_FLIP_NONE);
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}