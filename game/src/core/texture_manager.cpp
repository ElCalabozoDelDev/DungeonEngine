#include "core/texture_manager.hpp"
#include "SDL_image.h"

TextureManager* TextureManager::s_pInstance = 0;

SDL_Texture* TextureManager::load(std::string fileName, entt::registry& registry)
{
    auto& renderer = registry.ctx().get<SDL_Renderer*>();
    SDL_Surface* pTempSurface = IMG_Load(fileName.c_str());
    if(pTempSurface == 0)
    {
        return 0;
    }
    SDL_Texture* pTexture = SDL_CreateTextureFromSurface(renderer, pTempSurface);
    SDL_FreeSurface(pTempSurface);
    if(pTexture != 0)
    {
        return pTexture;
    }
    return 0;
}

void TextureManager::draw(entt::registry& registry, SDL_Texture *pTexture, int x, int y, int width, int height, SDL_RendererFlip flip)
{
    auto& pRenderer = registry.ctx().get<SDL_Renderer*>();
    SDL_Rect srcRect;
    SDL_Rect destRect;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = destRect.w = width;
    srcRect.h = destRect.h = height;
    destRect.x = x;
    destRect.y = y;
    
    SDL_RenderCopyEx(pRenderer, pTexture, &srcRect, &destRect, 0, 0, flip);
}

void TextureManager::drawFrame(entt::registry& registry, SDL_Texture *pTexture, int x, int y, int width, int height, int currentRow, int currentFrame, double angle, int alpha, SDL_RendererFlip flip)
{
    auto& pRenderer = registry.ctx().get<SDL_Renderer*>();
    SDL_Rect srcRect;
    SDL_Rect destRect;
    srcRect.x = width * currentFrame;
    srcRect.y = height * currentRow;
    srcRect.w = destRect.w = width;
    srcRect.h = destRect.h = height;
    destRect.x = x;
    destRect.y = y;
    
    SDL_SetTextureAlphaMod(pTexture, alpha);
    SDL_RenderCopyEx(pRenderer, pTexture, &srcRect, &destRect, angle, 0, flip);
}

void TextureManager::drawTile(entt::registry& registry, SDL_Texture *pTexture, int margin, int spacing, int x, int y, int width, int height, int currentRow, int currentFrame)
{
    auto& pRenderer = registry.ctx().get<SDL_Renderer*>();
    SDL_Rect srcRect;
    SDL_Rect destRect;
    srcRect.x = margin + (spacing + width) * currentFrame;
    srcRect.y = margin + (spacing + height) * currentRow;
    srcRect.w = destRect.w = width;
    srcRect.h = destRect.h = height;
    destRect.x = x;
    destRect.y = y;
    
    SDL_RenderCopyEx(pRenderer, pTexture, &srcRect, &destRect, 0, 0, SDL_FLIP_NONE);
}
