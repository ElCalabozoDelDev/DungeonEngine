#ifndef LEVEL_HPP
#define LEVEL_HPP

#include "world/layer.hpp"
#include "world/tile_set.hpp"
#include <vector>


class Level {
private:
	friend class LevelParser;
	std::vector<Tileset> m_tilesets;
	std::vector<Layer *> m_layers;
	Level(){}

public:
	~Level() {}
	void update();
	void render();
	std::vector<Tileset> *getTilesets() { return &m_tilesets; }
	std::vector<Layer *> *getLayers() { return &m_layers; }
};

#endif