#ifndef SPRITE_COMPONENT_HPP
#define SPRITE_COMPONENT_HPP

struct SpriteComponent {
    int spriteWidth;
    int spriteHeight;
    int spriteRow;  // Fila del sprite en la matriz
    int spriteCol;  // Columna del sprite en la matriz (actualmente visible)
    int currentSprite;  // Sprite actual de la animación
};

#endif // SPRITE_COMPONENT_HPP