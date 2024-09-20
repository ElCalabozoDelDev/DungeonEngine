#ifndef TILE_LAYER_HPP
#define TILE_LAYER_HPP

// #include "core/vector_2d.hpp"
#include "entt/entt.hpp"
#include <vector>

class TileLayer
{
public:
    
    TileLayer(){}
    
    virtual ~TileLayer() {}
    
    static void render(entt::registry& registry);
    
    static entt::entity getTilesetByID(entt::registry& registry, int tileID);
    

    
    
private:
    
};

#endif