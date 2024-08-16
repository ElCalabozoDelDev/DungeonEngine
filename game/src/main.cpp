#include "SDL.h"
#include "entt.hpp"
#include <iostream>

// Componentes
struct Position {
    float x, y;
};

struct Velocity {
    float vx, vy;
};

struct PlayerControlled {};

// Sistema de movimiento
void update_position(entt::registry& registry, float deltaTime) {
    auto view = registry.view<Position, Velocity>();
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& vel = view.get<Velocity>(entity);

        // Actualizar la posición
        pos.x += vel.vx * deltaTime;
        pos.y += vel.vy * deltaTime;

        // Verificar los límites de la ventana
        const int windowWidth = 800;  // Ancho de la ventana
        const int windowHeight = 600; // Alto de la ventana
        const int ballSize = 20;      // Tamaño de la bola

        // Limitar la posición en el eje X
        if (pos.x < 0) {
            pos.x = 0;
        } else if (pos.x + ballSize > windowWidth) {
            pos.x = windowWidth - ballSize;
        }

        // Limitar la posición en el eje Y
        if (pos.y < 0) {
            pos.y = 0;
        } else if (pos.y + ballSize > windowHeight) {
            pos.y = windowHeight - ballSize;
        }
    }
}

// Sistema de entrada del teclado
void handle_input(entt::registry& registry) {
    auto view = registry.view<Velocity, PlayerControlled>();

    for (auto entity : view) {
        auto& vel = view.get<Velocity>(entity);

        const Uint8* state = SDL_GetKeyboardState(nullptr);

        vel.vx = 0;
        vel.vy = 0;

        if (state[SDL_SCANCODE_UP]) {
            vel.vy = -200.0f;
        }
        if (state[SDL_SCANCODE_DOWN]) {
            vel.vy = 200.0f;
        }
        if (state[SDL_SCANCODE_LEFT]) {
            vel.vx = -200.0f;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            vel.vx = 200.0f;
        }
    }
}

// Sistema de renderizado
void render(SDL_Renderer* renderer, entt::registry& registry) {
    auto view = registry.view<Position>();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Blanco
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rojo
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        SDL_Rect ball = {static_cast<int>(pos.x), static_cast<int>(pos.y), 20, 20};
        SDL_RenderFillRect(renderer, &ball);
    }

    SDL_RenderPresent(renderer);
}

int WinMain( int argc, char* args[] )
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("EnTT + SDL2",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Error al crear la ventana: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error al crear el renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    entt::registry registry;

    // Crear la bola controlada por el jugador
    auto ball = registry.create();
    registry.emplace<Position>(ball, 390.0f, 290.0f);  // Posición inicial
    registry.emplace<Velocity>(ball, 0.0f, 0.0f);      // Velocidad inicial
    registry.emplace<PlayerControlled>(ball);          // Marca que esta bola es controlada por el jugador

    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handle_input(registry);        // Maneja la entrada del teclado
        update_position(registry, deltaTime); // Actualiza la posición
        render(renderer, registry);    // Renderiza la escena
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
