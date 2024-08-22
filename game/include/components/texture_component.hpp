#ifndef TEXTURE_COMPONENT_HPP
#define TEXTURE_COMPONENT_HPP
#include "SDL.h"

struct TextureComponent {
    SDL_Texture *texture;
    int spriteWidth;
    int spriteHeight;
    int spriteRow;  // Fila del sprite en la matriz
    int spriteCol;  // Columna del sprite en la matriz (actualmente visible)
};
#endif // TEXTURE_COMPONENT_HPP