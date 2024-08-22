#include "systems/render_system.hpp"
#include "components/position_component.hpp"
#include "components/texture_component.hpp"
#include "components/animation_component.hpp"

void RenderSystem::render(SDL_Renderer *renderer, entt::registry &registry) {
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
    SDL_Rect dstRect = {static_cast<int>(pos.x), static_cast<int>(pos.y),
                        spriteWidth, spriteHeight};

    // Renderizar la textura
    SDL_RenderCopy(renderer, tex.texture, &srcRect, &dstRect);
  }

  SDL_RenderPresent(renderer);
}