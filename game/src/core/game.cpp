#include "core/game.hpp"
#include <SDL.h>
#include <SDL_render.h>
#include <iostream>
#include <windows.h>

#include "core/config_loader.hpp"
#include "scene/entity_loader.hpp"

Game *Game::s_pInstance = nullptr;

Game::Game() : m_running(false), m_gWindow(nullptr), m_gRenderer(nullptr) {}

Game::~Game() {}

void Game::init(const char *xmlGamePath)
{
  m_xmlGamePath = xmlGamePath;
  // Cargar la configuración del juego desde un archivo XML
  ConfigLoader::loadConfigFromXML(m_xmlGamePath.c_str(), m_config);

  int flags = m_config.fullScreen ? SDL_WINDOW_FULLSCREEN : 0;
  m_frameDelay = 1000 /  m_config.frameRate;
  if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
  {
    m_gWindow = SDL_CreateWindow(m_config.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_config.screenWidth, m_config.screenHeight, flags);
    if (m_gWindow)
    {
      m_gRenderer = SDL_CreateRenderer(
          m_gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (m_gRenderer)
      {
        m_running = true;

        // Configuración de ImGui
        m_imguiSystem.init(m_gWindow, m_gRenderer);
      }
    }
  }
  else
  {
    std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
    m_running = false;
  }
  createEntities();
}

void Game::createEntities()
{
  // Carga entidades desde un archivo XML
  EntityLoader::loadPlayerDataFromXML(m_xmlGamePath, m_registry, m_gRenderer);
}

// # GAME LOOP
void Game::handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    m_imguiSystem.handleEvents(event);
    if (event.type == SDL_QUIT)
    {
      m_running = false;
    }
  }
  m_movementSystem.handle(m_registry); // Maneja el movimiento
}

void Game::update()
{
  Uint32 currentTime = SDL_GetTicks();
  static Uint32 lastTime = currentTime;
  float deltaTime = (currentTime - lastTime) / 1000.0f;
  lastTime = currentTime;

  m_transformSystem.update(m_registry, deltaTime);       // Actualiza la posición
  m_updateAnimationSystem.update(m_registry, deltaTime); // Actualiza la animación
}

void Game::render()
{

  SDL_RenderClear(m_gRenderer);
  m_imguiSystem.render(m_registry, m_gRenderer);

  m_renderSystem.render(m_gRenderer, m_registry);

  SDL_SetRenderDrawColor(m_gRenderer, 0, 0, 0, 255);

  // Mostrar el contenido renderizado en la pantalla
  SDL_RenderPresent(m_gRenderer);
}

void Game::clean()
{
  // Limpieza de ImGui
  m_imguiSystem.shutdown();

  SDL_DestroyRenderer(m_gRenderer);
  SDL_DestroyWindow(m_gWindow);
  SDL_Quit();
}

void Game::run()
{
  Uint32 frameStart, frameTime;
  while (isRunning())
  {
    frameStart = SDL_GetTicks();
    handleEvents();
    update();
    render();
    frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < m_frameDelay)
    {
      SDL_Delay(m_frameDelay - frameTime);
    }
  }

  clean();
}
// END GAME LOOP
