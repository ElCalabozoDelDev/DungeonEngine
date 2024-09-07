#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <vector>
#include "world/tile_set.hpp"
#include "world/layer.hpp"

class Level {
private:
	std::vector<Tileset> m_tilesets;
	std::vector<Layer*> m_layers;
public:
	Level();
	~Level() {}
	void update();
	void render();
	std::vector<Tileset>* getTilesets() 
	{ 
		return &m_tilesets;  
	}
	std::vector<Layer*>* getLayers() 
	{ 
		return &m_layers; 
	}
};

#endif