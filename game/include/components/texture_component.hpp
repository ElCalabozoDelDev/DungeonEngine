#ifndef TEXTURE_COMPONENT_HPP
#define TEXTURE_COMPONENT_HPP
#include "SDL.h"

struct TextureComponent {
    SDL_Texture *texture;
    int spriteRow;  // Fila del sprite en la matriz
    int spriteCol;  // Columna del sprite en la matriz (actualmente visible)
    int currentFrame;  // Frame actual de la animación
    int currentSprite;  // Sprite actual de la animación
    int totalFrames;  // Total de frames en la fila (en este caso, 3)
    float animationTime;  // Tiempo entre frames en milisegundos
    float timeSinceLastFrame;  // Tiempo acumulado desde el último cambio de frame
};
#endif // TEXTURE_COMPONENT_HPP