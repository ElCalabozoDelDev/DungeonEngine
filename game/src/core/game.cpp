#include "core/game.hpp"
#include "components/position.hpp"
#include "components/velocity.hpp"
#include "components/player_controlled.hpp"
#include "systems/update_position.hpp"
#include "systems/handle_input.hpp"
#include "systems/render_entities.hpp"
#include <iostream>

Game* Game::s_pInstance = nullptr;

Game::Game() : running(false), window(nullptr), renderer(nullptr)
{
}

Game::~Game()
{
}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
    int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if (window)
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                running = true;
            }
        }
    }
    else
    {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
        running = false;
    }

    // Aquí puedes inicializar tu juego, como la creación de la bola controlada por el jugador
    auto ball = registry.create();
    registry.emplace<Position>(ball, 390.0f, 290.0f);
    registry.emplace<Velocity>(ball, 0.0f, 0.0f);
    registry.emplace<PlayerControlled>(ball);
}

void Game::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running = false;
        }
    }
}

void Game::update()
{
    Uint32 currentTime = SDL_GetTicks();
    static Uint32 lastTime = currentTime;
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    handle_input(registry);        // Maneja la entrada del teclado
    update_position(registry, deltaTime); // Actualiza la posición
}

void Game::render()
{
    render_entities(renderer, registry);
}

void Game::clean()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::run()
{
    while (isRunning())
    {
        handleEvents();
        update();
        render();
    }

    clean();
}