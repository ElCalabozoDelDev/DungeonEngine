#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "imgui.h"

#include "components/player_controlled.hpp"
#include "components/position.hpp"
#include "components/velocity.hpp"
#include "core/game.hpp"
#include "systems/handle_input.hpp"
#include "systems/render_entities.hpp"
#include "systems/update_position.hpp"
#include <iostream>

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

  // Aquí puedes inicializar tu juego, como la creación de la bola controlada
  // por el jugador
  auto ball = registry.create();
  registry.emplace<Position>(ball, 390.0f, 290.0f);
  registry.emplace<Velocity>(ball, 0.0f, 0.0f);
  registry.emplace<PlayerControlled>(ball);
}

void Game::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      running = false;
    }
  }
}

void Game::update() {
  Uint32 currentTime = SDL_GetTicks();
  static Uint32 lastTime = currentTime;
  float deltaTime = (currentTime - lastTime) / 1000.0f;
  lastTime = currentTime;

  handle_input(registry);               // Maneja la entrada del teclado
  update_position(registry, deltaTime); // Actualiza la posición
}

void Game::render() { render_entities(gRenderer, registry); }

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
  Uint32 frameStart;
  int frameTime;
  while (isRunning()) {
    handleEvents();
    update();

    // Iniciar un nuevo frame de ImGui
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();

    // Crear una ventana de ImGui para mostrar la posición de la esfera roja
    auto view = registry.view<Position>();
    for (auto entity : view) {
      auto &pos = view.get<Position>(entity);

      ImGui::Begin("Posición de la Esfera Roja");
      ImGui::Text("X: %.2f", pos.x);
      ImGui::Text("Y: %.2f", pos.y);
      ImGui::End();
    }

    SDL_RenderClear(gRenderer);
    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), gRenderer);
    render();
    SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255);

    // Mostrar el contenido renderizado en la pantalla
    SDL_RenderPresent(gRenderer);
  }

  clean();
}
