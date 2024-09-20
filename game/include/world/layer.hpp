#ifndef LAYER_HPP
#define LAYER_HPP
#include "entt/entt.hpp"

class Level;

class Layer
{
public:
    virtual void render(entt::registry& registry) = 0;
    virtual void update() = 0;
    protected:
    virtual ~Layer() {}
};

#endif