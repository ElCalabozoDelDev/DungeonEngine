#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <memory>
#include <string>
#include "SDL_render.h"
#include <map>

class TextureManager
{
public:
    
    static TextureManager* Instance()
    {
        if(s_pInstance.get() == 0)
        {
            s_pInstance.reset(new TextureManager());
        }
        return s_pInstance.get();
    }
    
    bool load(std::string fileName, std::string id, SDL_Renderer* pRenderer);
    
    void clearTextureMap();
    void clearFromTextureMap(std::string id);
    
    void draw(std::string id, int x, int y, int width, int height, float zoomLevel, SDL_Renderer* pRenderer, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void drawFrame(std::string id, int x, int y, int width, int height, int currentRow, int currentFrame, SDL_Renderer* pRenderer, double angle, int alpha, float zoomLevel, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void drawTile(std::string id, int margin, int spacing, int x, int y, int width, int height, int currentRow, int currentFrame, float zoomLevel, SDL_Renderer *pRenderer);
    
    std::map<std::string, SDL_Texture*> getTextureMap() { return m_textureMap; }
    
    ~TextureManager() {}
private:
    TextureManager() {}
    
    
    TextureManager(const TextureManager&);
	TextureManager& operator=(const TextureManager&);
    
    std::map<std::string, SDL_Texture*> m_textureMap;
    
    static std::unique_ptr<TextureManager> s_pInstance;
};

#endif
