#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <string>
#include "SDL_render.h"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

class TextureManager
{
public:
    static TextureManager* Instance()
    {
        if(s_pInstance == 0)
        {
            s_pInstance = new TextureManager();
            return s_pInstance;
        }
       
        return s_pInstance;
    }
    
    SDL_Texture* load(std::string fileName, entt::registry& registry);
    
    void draw(entt::registry& registry, SDL_Texture *pTexture, int x, int y, int width, int height, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void drawFrame(entt::registry& registry, SDL_Texture *pTexture, int x, int y, int width, int height, int currentRow, int currentFrame, double angle, int alpha, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void drawTile(entt::registry& registry, SDL_Texture *pTexture, int margin, int spacing, int x, int y, int width, int height, int currentRow, int currentFrame);
    
private:
    
    TextureManager() {}
    ~TextureManager() {}
    
    TextureManager(const TextureManager&);
	TextureManager& operator=(const TextureManager&);
    
    static TextureManager* s_pInstance;
};

typedef TextureManager TheTextureManager;

#endif
