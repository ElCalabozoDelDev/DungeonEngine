#include "core/game.hpp"
#include "SDL_image.h"
#include "imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL_render.h>
#include <iostream>
#include <windows.h>

#include "components/player_component.hpp"
#include "components/position_component.hpp"
#include "components/texture_component.hpp"
#include "components/velocity_component.hpp"
#include "components/animation_component.hpp"

#include "scene/entity_loader.hpp"

Game *Game::s_pInstance = nullptr;

Game::Game() : m_running(false), m_gWindow(nullptr), m_gRenderer(nullptr), m_xmlGamePath("../assets/game.xml") {}

Game::~Game() {}

void Game::init(const char *title, int xpos, int ypos, int width, int height,
                bool fullscreen, int fps, int frameDelay)
{
  int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
  m_fps = fps;
  m_frameDelay = frameDelay;
  if (SDL_Init(SDL_INIT_VIDEO) == 0)
  {
    m_gWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
    if (m_gWindow)
    {
      m_gRenderer = SDL_CreateRenderer(
          m_gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (m_gRenderer)
      {
        m_running = true;

        // Configuración de ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |=
            ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        // Estilo
        ImGui::StyleColorsDark();

        // Configuración para SDL2
        ImGui_ImplSDL2_InitForSDLRenderer(m_gWindow, m_gRenderer);
        ImGui_ImplSDLRenderer2_Init(m_gRenderer);
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
    ImGui_ImplSDL2_ProcessEvent(&event);
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
  // Iniciar un nuevo frame de ImGui
  ImGui_ImplSDL2_NewFrame();
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui::NewFrame();

  // Crear una ventana de ImGui para mostrar la posición de la esfera roja
  auto view = m_registry.view<AnimationComponent, TextureComponent>();
  for (auto entity : view)
  {
    auto &animation = view.get<AnimationComponent>(entity);
    auto &texture = m_registry.get<TextureComponent>(entity);

    ImGui::Begin("Estado de la textura");
    ImGui::Text("spriteRow: %.2d", texture.spriteRow);
    ImGui::Text("spriteCol: %.2d", texture.spriteCol);
    ImGui::Text("currentSprite: %.2d", animation.currentSprite);
    ImGui::Text("currentFrame: %.2d", animation.currentFrame);
    ImGui::Text("totalFrames: %.2d", animation.totalFrames);
    ImGui::Text("animationTime: %.2f", animation.animationTime);
    ImGui::Text("timeSinceLastFrame: %.2f", animation.timeSinceLastFrame);
    ImGui::End();
  }

  SDL_RenderClear(m_gRenderer);
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_gRenderer);

  m_renderSystem.render(m_gRenderer, m_registry);

  SDL_SetRenderDrawColor(m_gRenderer, 0, 0, 0, 255);

  // Mostrar el contenido renderizado en la pantalla
  SDL_RenderPresent(m_gRenderer);
}

void Game::clean()
{
  // Limpieza de ImGui
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

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
