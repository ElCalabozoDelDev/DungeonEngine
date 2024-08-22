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


Game *Game::s_pInstance = nullptr;

Game::Game() : running(false), gWindow(nullptr), gRenderer(nullptr) {}

Game::~Game() {}

void Game::init(const char *title, int xpos, int ypos, int width, int height,
                bool fullscreen, int fps, int frameDelay) {
  int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
  m_fps = fps;
  m_frameDelay = frameDelay;
  if (SDL_Init(SDL_INIT_VIDEO) == 0) {
    gWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
    if (gWindow) {
      gRenderer = SDL_CreateRenderer(
          gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (gRenderer) {
        running = true;

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
        ImGui_ImplSDL2_InitForSDLRenderer(gWindow, gRenderer);
        ImGui_ImplSDLRenderer2_Init(gRenderer);
      }
    }
  } else {
    std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
    running = false;
  }

  createEntities();
}


SDL_Texture *Game::loadTexture(const std::string &path,
                               SDL_Renderer *renderer) {
  SDL_Texture *newTexture = IMG_LoadTexture(renderer, path.c_str());
  if (newTexture == nullptr) {
    std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
  }
  return newTexture;
}

void Game::createEntities() {
  // Aquí puedes inicializar tu juego, como la creación de la bola controlada
  // por el jugador
  SDL_Texture *playerTexture = loadTexture("../assets/Player/1-Heroes-Animated.png", gRenderer);
  auto player = registry.create();
  registry.emplace<PositionComponent>(player, 390.0f, 290.0f);
  registry.emplace<VelocityComponent>(player, 0.0f, 0.0f);
  registry.emplace<PlayerComponent>(player);
  registry.emplace<TextureComponent>(player, playerTexture, 2, 4, 0, 0, 3, 0.3, 0);
  if (player == entt::null) {
    std::cerr << "Error al crear la entidad del jugador" << std::endl;
  }
}

// # GAME LOOP
void Game::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }
  m_movementSystem.handle(registry); // Maneja el movimiento
}

void Game::update() {
  Uint32 currentTime = SDL_GetTicks();
  static Uint32 lastTime = currentTime;
  float deltaTime = (currentTime - lastTime) / 1000.0f;
  lastTime = currentTime;

  m_transformSystem.update(registry, deltaTime); // Actualiza la posición
  m_updateAnimationSystem.update(registry, deltaTime); // Actualiza la animación
}

void Game::render() {
  // Iniciar un nuevo frame de ImGui
  ImGui_ImplSDL2_NewFrame();
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui::NewFrame();

  // Crear una ventana de ImGui para mostrar la posición de la esfera roja
  auto view = registry.view<TextureComponent>();
  for (auto entity : view) {
    auto &texture = view.get<TextureComponent>(entity);

    ImGui::Begin("Estado de la textura");
    ImGui::Text("spriteRow: %.2d", texture.spriteRow);
    ImGui::Text("spriteCol: %.2d", texture.spriteCol);
    ImGui::Text("currentSprite: %.2d", texture.currentSprite);
    ImGui::Text("currentFrame: %.2d", texture.currentFrame);
    ImGui::Text("totalFrames: %.2d", texture.totalFrames);
    ImGui::Text("animationTime: %.2f", texture.animationTime);
    ImGui::Text("timeSinceLastFrame: %.2f", texture.timeSinceLastFrame);
    ImGui::End();
  }

  SDL_RenderClear(gRenderer);
  ImGui::Render();
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), gRenderer);

  m_renderSystem.render(gRenderer, registry);

  SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

  // Mostrar el contenido renderizado en la pantalla
  SDL_RenderPresent(gRenderer);
}

void Game::clean() {
  // Limpieza de ImGui
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
}

void Game::run() {
  Uint32 frameStart, frameTime;
  while (isRunning()) {
    frameStart = SDL_GetTicks();
    handleEvents();
    update();
    render();
    frameTime = SDL_GetTicks() - frameStart;
    if (frameTime < m_frameDelay) {
      SDL_Delay(m_frameDelay - frameTime);
    }
  }

  clean();
}
// END GAME LOOP
