#ifndef ANIMATION_COMPONENT_HPP
#define ANIMATION_COMPONENT_HPP

enum class AnimationLoopType
{
    Oscillating, // Animación de ida y vuelta
    Infinite,    // Repetición infinita
    Single       // Repetición de una sola vez
};

enum class InterpolationMode
{
    None,      // Sin interpolación
    Linear,    // Interpolación lineal
    EaseIn,    // Interpolación de aceleración
    EaseOut,   // Interpolación de desaceleración
    EaseInOut  // Interpolación de suavizado
};

struct AnimationComponent
{
    int currentFrame;  // Frame actual de la animación
    int totalFrames;  // Total de frames en la fila (en este caso, 3)
    float animationTime;  // Tiempo entre frames en milisegundos
    float timeSinceLastFrame;  // Tiempo acumulado desde el último cambio de frame
    float speedMultiplier = 1.0f;  // Multiplicador de velocidad de la animación
    float pauseDuration = 0.0f;  // Duración de la pausa en segundos
    float pauseTimer = 0.0f;  // Temporizador de pausa
    bool isReversing = false;  // Dirección de la animación (forward o reverse)
    bool isPaused = false;  // Indica si la animación está pausada
    bool pauseAtEnds = false;  // Indica si la animación debe pausarse al llegar al final
    AnimationLoopType loopType = AnimationLoopType::Oscillating;  // Tipo de bucle de la animación
    InterpolationMode interpolationMode = InterpolationMode::EaseInOut;  // Modo de interpolación
};

#endif // ANIMATION_COMPONENT_HPP