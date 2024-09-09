#include "systems/render_system.hpp"
#include "components/position_component.hpp"
#include "components/texture_component.hpp"
#include "components/animation_component.hpp"
#include <SDL_render.h>

void RenderSystem::run(entt::registry &registry) {
  SDL_Renderer *renderer = registry.ctx().get<SDL_Renderer *>();

  // Obtener la vista de entidades que tienen tanto PositionComponent como
  // TextureComponent
  auto view = registry.view<PositionComponent, TextureComponent, AnimationComponent>();

  for (auto entity : view) {
    auto &pos = view.get<PositionComponent>(entity);
    auto &tex = view.get<TextureComponent>(entity);
    auto &ani = view.get<AnimationComponent>(entity);

    int spriteWidth = tex.spriteWidth;
    int spriteHeight = tex.spriteHeight;

    // Calcular las coordenadas de origen basadas en el frame actual
    SDL_Rect srcRect;
    srcRect.x = ani.currentSprite * spriteWidth;
    srcRect.y = tex.spriteRow * spriteHeight;
    srcRect.w = spriteWidth;
    srcRect.h = spriteHeight;

    // Definir el rectángulo de destino
    SDL_Rect dstRect = {static_cast<int>(pos.position.getX()), static_cast<int>(pos.position.getY()),
                        spriteWidth, spriteHeight};

    // Renderizar la textura
    SDL_RenderCopy(renderer, tex.texture, &srcRect, &dstRect);
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

}