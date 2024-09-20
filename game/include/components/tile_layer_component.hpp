#ifndef TILE_LAYER_COMPONENT_HPP
#define TILE_LAYER_COMPONENT_HPP

#include "world/tile_layer.hpp"

struct TileLayerComponent
{
    TileLayer *tileLayer;
    int tileSize;
};

#endif
