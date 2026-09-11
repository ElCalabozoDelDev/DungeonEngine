#include "SDL_image.h"
#include <SDL_render.h>
#include <engine/graphics/texture_manager.hpp>
#include <iostream>

namespace de
{
std::unique_ptr<TextureManager> TextureManager::s_pInstance;

bool TextureManager::load(std::string fileName, std::string id,
                          SDL_Renderer* pRenderer)
{
    SDL_Surface* pTempSurface = IMG_Load(fileName.c_str());

    if (pTempSurface == 0)
    {
        std::cout << IMG_GetError();
        return false;
    }

    SDL_Texture* pTexture =
        SDL_CreateTextureFromSurface(pRenderer, pTempSurface);

    SDL_FreeSurface(pTempSurface);

    if (pTexture != 0)
    {
        m_textureMap[id] = pTexture;
        return true;
    }
    return false;
}

void TextureManager::draw(std::string id, int x, int y, int width, int height,
                          float zoomLevel, SDL_Renderer* pRenderer,
                          SDL_RendererFlip flip)
{
    SDL_Rect srcRect;
    SDL_Rect destRect;

    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = width;
    srcRect.h = height;
    destRect.x = x;
    destRect.y = y;
    destRect.w = width * zoomLevel;
    destRect.h = height * zoomLevel;

    SDL_RenderCopyEx(pRenderer, m_textureMap[id], &srcRect, &destRect, 0, 0,
                     flip);
}

void TextureManager::drawFrame(std::string id, int x, int y, int width,
                               int height, int currentRow, int currentFrame,
                               SDL_Renderer* pRenderer, double angle, int alpha,
                               float zoomLevel, SDL_RendererFlip flip)
{
    SDL_Rect srcRect;
    SDL_Rect destRect;
    srcRect.x = width * currentFrame;
    srcRect.y = height * currentRow;
    srcRect.w = width;
    srcRect.h = height;
    destRect.x = x;
    destRect.y = y;
    destRect.w = width * zoomLevel;
    destRect.h = height * zoomLevel;

    SDL_SetTextureAlphaMod(m_textureMap[id], alpha);
    SDL_RenderCopyEx(pRenderer, m_textureMap[id], &srcRect, &destRect, angle, 0,
                     flip);
}

void TextureManager::drawTile(std::string id, int margin, int spacing, int x,
                              int y, int width, int height, int currentRow,
                              int currentFrame, float zoomLevel,
                              SDL_Renderer* pRenderer)
{
    SDL_Rect srcRect;
    SDL_Rect destRect;

    srcRect.x = margin + (spacing + width) * currentFrame;
    srcRect.y = margin + (spacing + height) * currentRow;
    srcRect.w = width;
    srcRect.h = height;
    destRect.x = x;
    destRect.y = y;
    destRect.w = width * zoomLevel;
    destRect.h = height * zoomLevel;

    SDL_RenderCopyEx(pRenderer, m_textureMap[id], &srcRect, &destRect, 0, 0,
                     SDL_FLIP_NONE);
}

void TextureManager::clearTextureMap() { m_textureMap.clear(); }

void TextureManager::clearFromTextureMap(std::string id)
{
    m_textureMap.erase(id);
}

} // namespace de
