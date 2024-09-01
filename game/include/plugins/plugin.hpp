#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include "core/game_loop.hpp"

class Plugin {
public:
    virtual void mount(GameLoop& gameLoop) = 0;
    virtual ~Plugin() = default;
};

#endif