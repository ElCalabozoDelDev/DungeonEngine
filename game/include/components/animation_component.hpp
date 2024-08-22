#ifndef ANIMATION_COMPONENT_HPP
#define ANIMATION_COMPONENT_HPP

struct AnimationComponent
{
    int currentFrame;  // Frame actual de la animación
    int currentSprite;  // Sprite actual de la animación
    int totalFrames;  // Total de frames en la fila (en este caso, 3)
    float animationTime;  // Tiempo entre frames en milisegundos
    float timeSinceLastFrame;  // Tiempo acumulado desde el último cambio de frame
};


#endif // ANIMATION_COMPONENT_HPP