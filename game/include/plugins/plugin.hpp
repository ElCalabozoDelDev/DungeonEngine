#ifndef PLUGIN_HPP
#define PLUGIN_HPP

class GameLoop;

class Plugin {
public:
    virtual void mount(GameLoop& gameLoop) = 0;
    virtual ~Plugin() = default;
};

#endif