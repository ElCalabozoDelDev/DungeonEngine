#ifndef GAME_HPP
#define GAME_HPP

#include "SDL.h"
#include "entt.hpp"

class Game
{
private:
    static Game* s_pInstance;
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    entt::registry registry;
    int m_fps;
    int m_frameDelay;
    Game();
    ~Game();

public:
    static Game* Instance()
    {
        if (s_pInstance == nullptr)
        {
            s_pInstance = new Game();
        }
        return s_pInstance;
    }

    void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen, int fps, int frameDelay);
    void handleEvents();
    void update();
    void render();
    void clean();
    void run();
    bool isRunning() const { return running; }
};


#endif // GAME_HPP